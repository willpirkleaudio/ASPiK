// -----------------------------------------------------------------------------
//    ASPiK Custom Views File:  customviews.cpp
//
/**
    \file   customviews.cpp
    \author Will Pirkle
    \date   17-September-2018
    \brief  implementation file for example ASPiK custom view and custom sub-controller
    		objects
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#include "customviews.h"

namespace VSTGUI {

/**
\brief WaveView constructor

\param size - the control rectangle
\param listener - the control's listener (usuall PluginGUI object)
\param tag - the control ID value
*/
WaveView::WaveView(const VSTGUI::CRect& size, IControlListener* listener, int32_t tag)
: CControl(size, listener, tag)
, ICustomView()
{
    // --- create circular buffer that is same size as the window is wide
	circularBuffer = new double[(int)size.getWidth()];

    // --- init
	writeIndex = 0;
	readIndex = 0;
	circularBufferLength = (int)size.getWidth();
    memset(circularBuffer, 0, circularBufferLength*sizeof(double));
	paintXAxis = true;
	currentRect = size;

    // --- ICustomView
    // --- create our incoming data-queue
    dataQueue = new moodycamel::ReaderWriterQueue<double, DATA_QUEUE_LEN>;
}

WaveView::~WaveView()
{
    if(circularBuffer)
        delete [] circularBuffer;

    if(dataQueue)
        delete dataQueue;
}

void WaveView::pushDataValue(double data)
{
    if(!dataQueue) return;

    // --- add data point, make room if needed
    dataQueue->enqueue(data);
}

void WaveView::updateView()
{
    // --- get the max value that was added to the queue during the last
    //     GUI timer ping interval
    double audioSample = 0.0;
    double max = 0.0;
    bool success = dataQueue->try_dequeue(audioSample);
    if(success)
    {
        max = audioSample;
        while(success)
        {
            success = dataQueue->try_dequeue(audioSample);
            if(success && audioSample > max)
                max = audioSample;
        }

        // --- add to circular buffer
        addWaveDataPoint(fabs(max));
    }

    // --- this will set the dirty flag to repaint the view
    invalid();
}

void WaveView::addWaveDataPoint(float fSample)
{
	if(!circularBuffer) return;
	circularBuffer[writeIndex] = fSample;
	writeIndex++;
	if(writeIndex > circularBufferLength - 1)
		writeIndex = 0;
}

void WaveView::clearBuffer()
{
	if(!circularBuffer) return;
	memset(circularBuffer, 0, circularBufferLength*sizeof(float));
	writeIndex = 0;
	readIndex = 0;
}

void WaveView::draw(CDrawContext* pContext)
{
    // --- setup the backround rectangle
    int frameWidth = 1;
    int plotLineWidth = 1;
    pContext->setLineWidth(frameWidth);
    pContext->setFillColor(CColor(200, 200, 200, 255)); // light grey
    pContext->setFrameColor(CColor(0, 0, 0, 255)); // black
	CRect size = getViewSize();

    // --- draw the rect filled (with grey) and stroked (line around rectangle)
    pContext->drawRect(size, kDrawFilledAndStroked);

    // --- this will be the line color when drawing lines
    //     alpha value is 200, so color is semi-transparent
    pContext->setFrameColor(CColor(32, 0, 255, 200));
    pContext->setLineWidth(plotLineWidth);

    if(!circularBuffer) return;

    // --- step through buffer
    int index = writeIndex - 1;
    if(index < 0)
        index = circularBufferLength - 1;

    for(int i=1; i<circularBufferLength; i++)
    {
        double sample = circularBuffer[index--];

        double normalized = sample*(double)size.getHeight();
        if(normalized > size.getHeight() - 2)
            normalized = (double)size.getHeight();

        // --- so there is an x-axis even if no data
        if(normalized == 0) normalized = 0.1f;

        if (paintXAxis)
        {
            const CPoint p1(size.left + i, size.bottom - size.getHeight() / 2.f);
            const CPoint p2(size.left + i, size.bottom - size.getHeight() / 2.f - 1.0);
            const CPoint p3(size.left + i, size.bottom - size.getHeight() / 2.f + 1.0);

            // --- move and draw lines
            pContext->drawLine(p1, p2);
#ifndef MAC
            pContext->drawLine(p1, p3); // MacOS render is a bit different, this just makes them look consistent
#endif
        }

        // --- halves
        normalized /= 2.f;

        // --- find the three points of interest
        const CPoint p1(size.left + i, size.bottom - size.getHeight()/2.f);
        const CPoint p2(size.left + i, size.bottom - size.getHeight()/2.f - normalized);
        const CPoint p3(size.left + i, size.bottom - size.getHeight()/2.f + normalized);

        // --- move and draw lines
        pContext->drawLine(p1, p2);
        pContext->drawLine(p1, p3);

        // --- wrap the index value if needed
        if(index < 0)
            index = circularBufferLength - 1;
    }
}

#ifdef HAVE_FFTW
/**
\brief SpectrumView constructor

\param size - the control rectangle
\param listener - the control's listener (usuall PluginGUI object)
\param tag - the control ID value
*/
SpectrumView::SpectrumView(const VSTGUI::CRect& size, IControlListener* listener, int32_t tag)
: CControl(size, listener, tag)
{
    // --- ICustomView
    // --- create our incoming data-queue
    dataQueue = new moodycamel::ReaderWriterQueue<double,FFT_LEN>;

    // --- double buffers for mag FFTs
    fftMagBuffersReady = new moodycamel::ReaderWriterQueue<double*,2>;
    fftMagBuffersEmpty = new moodycamel::ReaderWriterQueue<double*,2>;

    // --- load up the empty queue with buffers
    fftMagBuffersEmpty->enqueue(fftMagnitudeArray_A);
    fftMagBuffersEmpty->enqueue(fftMagnitudeArray_B);

    // --- buffer being drawn, only ever used by draw code
    currentFFTMagBuffer = nullptr;

    // --- FFTW inits
    data        = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FFT_LEN);
    fft_result  = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FFT_LEN);
    ifft_result = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FFT_LEN);

    plan_forward  = fftw_plan_dft_1d(FFT_LEN, data, fft_result, FFTW_FORWARD, FFTW_ESTIMATE);
    plan_backward = fftw_plan_dft_1d(FFT_LEN, fft_result, ifft_result, FFTW_BACKWARD, FFTW_ESTIMATE);

    // --- window
    setWindow(spectrumViewWindowType::kBlackmanHarrisWindow);
}

SpectrumView::~SpectrumView()
{
    fftw_destroy_plan( plan_forward );
    fftw_destroy_plan( plan_backward );

    fftw_free( data );
    fftw_free( fft_result );
    fftw_free( ifft_result );

	if (dataQueue)
		delete dataQueue;

	if (fftMagBuffersReady)
		delete fftMagBuffersReady;

	if (fftMagBuffersEmpty)
		delete fftMagBuffersEmpty;
}

void SpectrumView::setWindow(spectrumViewWindowType _window)
{
    window = _window;
    memset(&fftWindow[0], 0, FFT_LEN*sizeof(double));

    // --- rectangular has fftWindow[0] = 0, fftWindow[FFT_LEN-1] = 0, all other points = 1.0
    if(window == spectrumViewWindowType::kRectWindow)
    {
        for (int n=0;n<FFT_LEN-1;n++)
            fftWindow[n] = 1.0;
    }
    else if(window == spectrumViewWindowType::kHannWindow)
    {
        for (int n=0;n<FFT_LEN;n++)
            fftWindow[n] = (0.5 * (1-cos((n*2.0*M_PI)/FFT_LEN)));
    }
    else if(window == spectrumViewWindowType::kBlackmanHarrisWindow)
    {
        for (int n=0;n<FFT_LEN;n++)
            fftWindow[n] = (0.42323 - (0.49755*cos((n*2.0*M_PI)/FFT_LEN))+ 0.07922*cos((2*n*2.0*M_PI)/FFT_LEN));
    }
    else // --- default to rectangular
    {
        for (int n=1;n<FFT_LEN-1;n++)
            fftWindow[n] = 1.0;
    }
}

bool SpectrumView::addFFTInputData(double inputSample)
{
    if(fftInputCounter >= FFT_LEN)
        return false;

    data[fftInputCounter][0] = inputSample*fftWindow[fftInputCounter]; // stick your audio samples in here
    data[fftInputCounter][1] = 0.0; // use this if your data is complex valued

    fftInputCounter++;
    if(fftInputCounter == FFT_LEN)
        return true; // ready for FFT

    return false;
}


void SpectrumView::pushDataValue(double data)
{
    if(!dataQueue) return;

    // --- add data point, make room if needed
    dataQueue->enqueue(data);
}

void SpectrumView::updateView()
{
    // --- grab samples from incoming queue and add to FFT input
    double audioSample = 0.0;
    bool success = dataQueue->try_dequeue(audioSample);
    if(!success) return;
    bool fftReady = false;
    while(success)
    {
        // --- keep adding values into the array; it will stop when full
        //     and return TRUE if the FFT buffer is full and we are ready
        //     to do a FFT
        if(addFFTInputData(audioSample))
            fftReady = true; // sticky flag

        // --- for this wave view, we can only show the FFT of the last
        //     512 points anyway, so we just keep popping them from the queue
        success = dataQueue->try_dequeue(audioSample);
    }

    if(fftReady)
    {
        // do the FFT
        fftw_execute(plan_forward);

        double* bufferToFill = nullptr;
        fftMagBuffersEmpty->try_dequeue(bufferToFill);

        if(!bufferToFill)
        {
            fftReady = false;
            fftInputCounter = 0;
            return;
        }

        int maxIndex = 0;
        for(int i=0; i<FFT_LEN; i++)
        {
            bufferToFill[i] = (getMagnitude(fft_result[i][0], fft_result[i][1]));
        }

        // --- normalize the FFT buffer for max = 1.0 (note this is NOT dB!!)
        normalizeBufferGetFMax(bufferToFill, 512, &maxIndex);

        // 1) homework = do plot in dB
        // 2) homework = add other windows
        // 3) homework = plot frequency on log x-axis (HARD!)

        // --- add the new FFT buffer to the queue
        fftMagBuffersReady->enqueue(bufferToFill);

        // --- set flags (can reduce number of flags?)
        fftReady = false;
        fftInputCounter = 0;
    }

    // --- this will set the dirty flag to repaint the view
    invalid();
}

void SpectrumView::draw(CDrawContext* pContext)
{
    // --- setup the backround rectangle
    int frameWidth = 1;
    int plotLineWidth = 1;
    pContext->setLineWidth(frameWidth);
    pContext->setFillColor(CColor(200, 200, 200, 255)); // light grey
    pContext->setFrameColor(CColor(0, 0, 0, 255)); // black

    // --- draw the rect filled (with grey) and stroked (line around rectangle)
	CRect size = getViewSize();
    pContext->drawRect(size, kDrawFilledAndStroked);

    // --- this will be the line color when drawing lines
    //     alpha value is 200, so color is semi-transparent
    pContext->setFrameColor(CColor(32, 0, 255, 200));
    pContext->setLineWidth(plotLineWidth);

    // --- is there a new fftBuffer?
    if(fftMagBuffersReady->peek())
    {
        // --- put current buffer in empty queue
        if(currentFFTMagBuffer)
            fftMagBuffersEmpty->try_enqueue(currentFFTMagBuffer);

        // --- get next buffer to plot
        fftMagBuffersReady->try_dequeue(currentFFTMagBuffer);
    }

    if(!currentFFTMagBuffer)
        return;

    // --- plot the FFT data
    double step = 128.0/size.getWidth();
    double magIndex = 0.0;

    // --- plot first point
    double yn = currentFFTMagBuffer[0];
    double ypt = size.bottom - size.getHeight()*yn;

    // --- make sure we leave room for bottom of frame
	if (ypt > size.bottom - frameWidth)
		ypt = size.bottom - frameWidth;

    // --- setup first point, which is last point for loop below
    CPoint lastPoint(size.left, ypt);

    for (int x = 1; x < size.getWidth()-1; x++)
    {
        // --- increment stepper for mag array
        magIndex += step;

        // --- interpolate to find magnitude at this step
        yn = interpArrayValue(currentFFTMagBuffer, 128, magIndex);

        // --- calculate top (y) value of point
        ypt = size.bottom - size.getHeight()*yn;

        // --- make sure we leave room for bottom of frame
		if (ypt > size.bottom - frameWidth)
			ypt = size.bottom - frameWidth;

        // --- create a graphic point to this location
        const CPoint p2(size.left + x, ypt);

        // --- filled FFT is a set of vertical lines that touch
        if(filledFFT)
        {
            // --- find bottom point
			CPoint bottomPoint(size.left + x, size.bottom - frameWidth);

            // --- draw vertical line
            pContext->drawLine(bottomPoint, p2);
        }
        else // line-FFT
        {
            // --- move and draw line segment
            pContext->drawLine(lastPoint, p2);

            // --- save for next segment
            lastPoint.x = p2.x;
            lastPoint.y = p2.y;
        }
    }
}

#endif

/**
\brief CustomKnobView constructor

\param size - the control rectangle
\param listener - the control's listener (usuall PluginGUI object)
\param tag - the control ID value
\param subPixmaps - the number of frames in the strip animation
\param heightOfOneImage- the number of frames in the strip animation
\param background- the graphics file for the control
\param offset- positional offset value
\param bSwitchKnob - flag to enable switch knob
*/
CustomKnobView::CustomKnobView (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint &offset, bool bSwitchKnob)
: CAnimKnob (size, listener, tag, subPixmaps, heightOfOneImage, background, offset)
{
    // --- ICustomView
    // --- create our incoming data-queue
    dataQueue = new moodycamel::ReaderWriterQueue<CustomViewMessage, 32>;
}

CustomKnobView::~CustomKnobView(void)
{
    if(dataQueue) delete dataQueue;
}

void CustomKnobView::sendMessage(void* data)
{
    CustomViewMessage* viewMessage = (CustomViewMessage*)data;
    
    // --- example of messaging: plugin core send message, we acknowledge
    if (viewMessage->message == MESSAGE_QUERY_CONTROL)
    {
        if (viewMessage->queryString.compare("Hello There!") == 0)
        {
            viewMessage->replyString.assign("I'm Here!!");
            viewMessage->messageData = this; // <– example of VERY risky thing to do; not recommended
        }
    }

    // --->> CustomViewMessage has =operator
    dataQueue->enqueue(*viewMessage);
}

void CustomKnobView::updateView()
{
    CustomViewMessage viewMessage;
    bool success = dataQueue->try_dequeue(viewMessage);
    while(success)
    {
        // --- not connected, but example of setting control's appearance via message
        //     you could use this to also show/hide the control, or move it to a new location
        if(viewMessage.message == MESSAGE_SET_CONTROL_ALPHA)
        {
            this->setAlphaValue(viewMessage.controlAlpha);
        }

        // --- keep popping the queue in case there were multiple message insertions
        success = dataQueue->try_dequeue(viewMessage);
    }

    // --- force redraw
    invalid();
}



}
