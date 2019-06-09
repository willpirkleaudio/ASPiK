// -----------------------------------------------------------------------------
//    ASPiK Custom Views File:  customviews.h
//
/**
    \file   customviews.h
    \author Will Pirkle
    \date   17-September-2018
    \brief  interface file for example ASPiK custom view and custom sub-controller
    		objects, includes waveform view, FFT spectrum view, a custom Knob
    		control, and a custom sub-controller that links a set of knobs together
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#pragma once
#include "vstgui/vstgui.h"
#include "vstgui/vstgui_uidescription.h" // for IController

#include "../PluginKernel/pluginstructures.h"

namespace VSTGUI {

// --- with an update cycle of ~50mSec, we need at least 2205 samples; this should be more than enough
const int DATA_QUEUE_LEN = 4096;

/**
\class WaveView
\ingroup Custom-Views
\brief
This object displays an audio histogram waveform view.\n

WaveView:
- uses a lock-free ring buffer for queueing up input data from the plugin
- implements ICustomView::pushDataValue() and ICustomView::updateView()
- the updateData() function finds the largest value that was pushed into
the data queue and adds that to the waveform buffer (circular)
- uses a circular buffer to make waveform appear to scroll
- each new input point pushes oldest sample out of the buffer

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class WaveView : public CControl, public ICustomView
{
public:
    WaveView(const CRect& size, IControlListener* listener, int32_t tag);
    ~WaveView();

    /** ICustomView method: this repaints the control */
    virtual void updateView() override;

	/** ICustomView method: push a new audio sample into the ring buffer */
	virtual void pushDataValue(double data) override;

	/** add a new point to the circular buffer for painting
	\param fSample the absolute value of the sample
	*/
	void addWaveDataPoint(float fSample);

	/** reset the circular buffer for a new run
	*/
	void clearBuffer();

	/** toggles showng of x axis
	\param _paintXAxis enable/disable functionality
	*/
	void showXAxis(bool _paintXAxis) { paintXAxis = _paintXAxis; }

	/** override of drawing function
	\param pContext incoming draw context
	*/
	void draw(CDrawContext* pContext) override;

    // --- for CControl pure abstract functions
	CLASS_METHODS(WaveView, CControl)

protected:
    // --- turn on/off zerodB line
    bool paintXAxis = true; ///< flag for painting X Axis

    // --- circular buffer and index values
    double* circularBuffer = nullptr;	///< circular buffer to store peak values
    int writeIndex = 0;		///< circular buffer write location
    int readIndex = 0;		///< circular buffer read location
    int circularBufferLength = 0;///< circular buffer length
	CRect currentRect;		///< the rect to draw into

private:
    // --- lock-free queue for incoming data, sized to DATA_QUEUE_LEN in length
    moodycamel::ReaderWriterQueue<double, DATA_QUEUE_LEN>* dataQueue = nullptr; ///< lock-free queue for incoming data, sized to DATA_QUEUE_LEN in length

};

#ifdef HAVE_FFTW
// --- FFTW (REQUIRED)
#include "fftw3.h"

/**
\enum spectrumViewWindowType
\ingroup Constants-Enums
\brief
Use this strongly typed enum to easily set the window type for the spectrum view

- enum class spectrumViewWindowType {kRectWindow, kHannWindow, kBlackmanHarrisWindow};

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
enum class spectrumViewWindowType {kRectWindow, kHannWindow, kBlackmanHarrisWindow};

// --- change this for higher accuracy; needs to be power of 2
const int FFT_LEN = 512;

// --- SpectrumView
/*
*/
/**
\class SpectrumView
\ingroup Custom-Views
\brief
This object displays the FFT of the incoming data.\n

SpectrumView:
- uses a lock-free ring buffer for queueing up input data from the plugin
- implements ICustomView::pushDataValue() and ICustomView::updateData()
- uses a pair of lock-free ring buffers to implement a safe double-buffering system
- during updates, the queue is dumped into the FFT array
- when a new FFT is processed, its magnitude array is calculated in the first
available buffer in the empty queue; it is then placed in the filled (ready) queue
- the draw() function is on the same thread with the PluginKernel/PluginGUI paradigm\
but just in case it ISN'T, the drawing uses the double buffer to safely get the
next available magnitude array to display
- the result is a super fast visually synchronized display

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class SpectrumView : public CControl, public ICustomView
{
public:
    SpectrumView(const CRect& size, IControlListener* listener, int32_t tag);
    ~SpectrumView();

	/** ICustomView method: this repaints the control */
	virtual void updateView() override;

	/** ICustomView method: push a new audio sample into the ring buffer */
	virtual void pushDataValue(double data) override;

	/** show FFT as filled (or unfilled) plot */
	void showFilledFFT(bool _filledFFT) { filledFFT = _filledFFT; }

	/** set the window
	\param _window the window type (seee windowType)
	*/
	void setWindow(spectrumViewWindowType _window);

	/** override to draw, called if the view should draw itself*/
	void draw(CDrawContext* pContext) override;

    // --- for CControl pure abstract functions
    CLASS_METHODS(SpectrumView, CControl)

protected:
    // --- for windowing; this doesn't need to be saved in current
    //     implementation but you may need it for homework/upgrading the object
	spectrumViewWindowType window = spectrumViewWindowType::kRectWindow; ///< window type

    // --- setup FFTW
    fftw_complex* data = nullptr;			///< fft input data
	fftw_complex* fft_result = nullptr;		///< fft output data
	fftw_complex* ifft_result = nullptr;	///< ifft output (not used)
	fftw_plan plan_forward;					///< plan for FFT
	fftw_plan plan_backward;				///< plan for IFFT (not used)

    // --- for FFT data input
    int fftInputCounter = 0;				///< input counter for FFT

 	/** returns true if the input buffer is full and ready for a FFT operation \n
	will keep track of indexing and reject samples when full
	\param inputSample input point for FFT
	*/
    bool addFFTInputData(double inputSample);

    // --- a double buffer pair of magnitude arrays
    double fftMagnitudeArray_A[FFT_LEN] = {0.0}; ///< 1/2 of double buffer (yes this is overkill but showing as demonstration!!)
    double fftMagnitudeArray_B[FFT_LEN] = {0.0}; ///< 1/2 of double buffer (yes this is overkill but showing as demonstration!!)

    // --- buffer for the assigned window
    double fftWindow[FFT_LEN] = {1.0}; ///< window buffer

    // --- pointer to mag buffer that drawing thread uses; note that
    //     this pointer is never shared with any other function
    double* currentFFTMagBuffer = nullptr; ///< poitner to current FFT buffer

    // --- NOTE: move these to another file for use by other objects!!
    // --- helper for FFT magnitude
    inline double getMagnitude(double re, double im)
    {
        return sqrt((re*re)+(im*im));
    }

	/** normalize a buffer and find the index of the maximum value at the same time
	\param buffer - pointer to buffer to normalize
	\param bufferSize - length of buffer
	\param ptrMaxIndex - return value passed by pointer of INDEX of max value in buffer
	*/
	inline double normalizeBufferGetFMax(double* buffer, unsigned int bufferSize, int* ptrMaxIndex)
    {
        double max = 0;
        double maxRetValue = 0;
        *ptrMaxIndex = 0;

        for(int j=0; j<bufferSize; j++)
        {
            if((fabs(buffer[j])) > max)
            {
                max = fabs(buffer[j]);
                *ptrMaxIndex = j;
            }
        }

        if(max > 0)
        {
            for(int j=0; j<bufferSize; j++)
            {
                buffer[j] = buffer[j]/max;
                if(j == *ptrMaxIndex)
                    maxRetValue = buffer[j];
            }
        }

        return maxRetValue;
    }

	/** interpolate a value from an array
	\param array - pointer to array to interpolate
	\param arraySize - length of array
	\param fractionalIndex - fractional location of value in array (e.g. 3.446 is 0.446 between 3 and 4)
	*/
	inline double interpArrayValue(double* array, int arraySize, double fractionalIndex)
    {
        // --- extract [index_x] values
        int x1 = (int)fractionalIndex;
        int x2 = x1 + 1;

        // --- check invalid conditions
        if(x1 >= arraySize)
            return 0.0;
        if(x2 >= arraySize)
            return array[x1];
        if(x2 - x1 == 0)    // 0 slope: should not ever happen
            return array[x1];

        // --- calculate decimal position of x
        double dx = (fractionalIndex - x1)/(x2 - x1);

        // --- use weighted sum method of interpolating
        return dx*array[x2] + (1-dx)*array[x1];
    }

protected:
    // --- filled/unfilled FFT
    bool filledFFT = true; ///< flag for filled FFT

private:
    // --- lock-free queue for incoming data, sized to FFT_LEN in length
    moodycamel::ReaderWriterQueue<double,FFT_LEN>* dataQueue = nullptr; ///< lock free ring buffer

    // --- a pair of lock-free queues to store empty and full magnitude buffers
    //     these are setup as double buffers but you can easily extend them
    //     to quad (4) and octal (8) if you want
    moodycamel::ReaderWriterQueue<double*,2>* fftMagBuffersReady = nullptr; ///< example of queuing system (yes I know it is overkill here)
    moodycamel::ReaderWriterQueue<double*,2>* fftMagBuffersEmpty = nullptr; ///< example of queuing system (yes I know it is overkill here)
};
#endif // defined FFTW


// --- custom view example
const unsigned int MESSAGE_SHOW_CONTROL = 0;
const unsigned int MESSAGE_HIDE_CONTROL = 1;
const unsigned int MESSAGE_SET_CONTROL_ALPHA = 2;
const unsigned int MESSAGE_QUERY_CONTROL = 3;

// --- example of a custom view message; here we control the visual appearance of a control
/**
\struct CustomViewMessage
\ingroup Custom-Views
\brief
Custom structure for passing messages and data to and from the plugin core object. See the Custom View tutorial project for more informaiton.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct CustomViewMessage
{
    CustomViewMessage() {}
    CustomViewMessage(const CustomViewMessage& initMessage)
    {
        message = initMessage.message;
        visible = initMessage.visible;
        showAlternateGraphic = initMessage.showAlternateGraphic;
        controlAlpha = initMessage.controlAlpha;
        queryString = initMessage.queryString;
        replyString = initMessage.replyString;
        messageData = initMessage.messageData;
    }
    
    CustomViewMessage& operator =(const CustomViewMessage& viewMessage)
    {
        message = viewMessage.message;
        visible = viewMessage.visible;
        showAlternateGraphic = viewMessage.showAlternateGraphic;
        controlAlpha = viewMessage.controlAlpha;
        queryString = viewMessage.queryString;
        replyString = viewMessage.replyString;
        messageData = viewMessage.messageData;
        return *this;
    }
    
    // --- show/hide flag
    unsigned int message = MESSAGE_HIDE_CONTROL;
    
    bool visible = true;
    bool showAlternateGraphic = false;
    double controlAlpha = 1.0; // transparency: 0 = invisible (100% transparent) and 1 = solidly visible (0% transparent)
    std::string queryString;
    std::string replyString;
    void* messageData = nullptr;
};

/**
\class CustomKnobView
\ingroup Custom-Views
\brief
This object demonstrates how to subclass an existing VSTGUI4 control to setup a communcation channel with it
using the ICustomView interface.\n

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class CustomKnobView : public CAnimKnob, public ICustomView
{
public:
    CustomKnobView(const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps,
            CCoord heightOfOneImage, CBitmap* background, const CPoint &offset,
            bool bSwitchKnob = false);

	/** ICustomView method: this repaints the control */
	virtual void updateView() override;

	/** ICustomView method: send a message to the object (!) */
	virtual void sendMessage(void* data) override;

protected:
    virtual ~CustomKnobView(void);

private:
    // --- lock-free queue for incoming data, sized to 32 in length
    moodycamel::ReaderWriterQueue<CustomViewMessage, 32>* dataQueue = nullptr; ///< lock-free queue for incoming data, sized to 32 in length
};


/**
\class KnobLinkController
\ingroup Custom-SubControllers
\brief
This object demonstrates how to create a sub-controller. In this object, you can link
multiple knob controls together so that moving one control moves all of them. There
is an associated button used to enable/disable the linking operation.

-	Subcontroller that links some number of CAnimKnobs so that when linked
turning one knobs turns all of the rest of the linked knobs regardless
of their control tags

-	use the sub-controller string "KnobLinkController" for the view
view container that holds the knobs to link; there can only be one
KnobLinkController for each view container

-	To hard-wire the linkage, set the flag in the constructor

-	To make linkage variable, a CTextButton must be used as the switcher; you can
easily change that by changing the code involving the linkControl variable

-	the verifyView( ) function gets called once per child view of the outer
continer; this is where we save the linker control (button) and push the
CAnimKnob controls onto our vector

-	the valueChanged( ) function gets called when *any* of the view containers sub
controls are moved; we only stash and control the CAnimKnobs so you can mix
other controls in the same view container that won't be affected

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class KnobLinkController : public IController
{
public:
	/** KnobLinkController constructor
	\param _parentController - pointer to the parent controller so we can forward messages to it
	*/
	KnobLinkController(IController* _parentController)
	{
		// --- save the parent listener
		parentController = _parentController;

		// --- INITIALIZE LINK STATE
		linkControls = false;
	}
	~KnobLinkController()
	{
		linkedKnobs.clear();
	}

	/** test to see if control is in the list of linked controls
	\param control - control to test
	\return true if control is in list, false otherwise
	*/
	bool isLinkedControl(CControl* control)
	{
		return std::find(linkedKnobs.begin(), linkedKnobs.end(), control) != linkedKnobs.end();
	}

	/** called once per child view of the container that owns the sub-controller this is where we grab and store the
	    view objects that we want to link
		\param view - the newly created view to use
		\param attributes - UIAttributes of control
		\param IUIDescription - IUIDescription of control
		\return the verified view
	*/
	virtual CView* verifyView(CView* view, const UIAttributes& attributes, const IUIDescription* description) override
	{
		CAnimKnob* knob = dynamic_cast<CAnimKnob*>(view);
		CTextButton* button = dynamic_cast<CTextButton*>(view);

		// --- save button, push back knob onto list
		if (button)
		{
			linkControl = button;
			if (button->getValueNormalized() != 0)
				linkControls = true;
			else
				linkControls = false;
		}
		else if (knob)
			linkedKnobs.push_back(knob);

		return view;
	}

	/** called when any control  in the view container changes; we only care about the link button and the knobs, we ignore the others
	view objects that we want to link
	\param control - the control whose value changed
	*/
	virtual void valueChanged(CControl* control) override
	{
		// --- set the link flag
		if (control == linkControl)
		{
			if (control->getValueNormalized() != 0)
				linkControls = true;
			else
				linkControls = false;

			return parentController->valueChanged(control);
		}

		// --- check flag
		if (!linkControls)
			return parentController->valueChanged(control);

		// --- we are linking
		//
		// --- make sure this is not a rogue control
		if (isLinkedControl(control))
		{
			// --- iterate list
			for (std::vector<CAnimKnob*>::iterator it = linkedKnobs.begin(); it != linkedKnobs.end(); ++it)
			{
				// --- set the control value for all knobs except the one generating this message
				CControl* ctrl = *it;

				if (ctrl && control != ctrl)
				{
					// --- set the control visually
					ctrl->setValueNormalized(control->getValueNormalized());

					// --- do the value change at parent level, to set on plugin
					parentController->valueChanged(ctrl);
                    
                    ctrl->invalid();
				}
			}
		}
		// --- do the value change at parent level, to set on plugin
		parentController->valueChanged(control);
	}

	/** called once per child view of the container - we simply forward the call to the parent listener
	\param attributes - UIAttributes of control
	\param IUIDescription - IUIDescription of control
	\return the verified view
	*/
	virtual CView* createView(const UIAttributes& attributes, const IUIDescription* description) override { return parentController->createView(attributes, description); }

	/** called when the user begins to edit a control (mouse click) - we simply forward the call to the parent listener
	\param pControl - the control
	*/
	virtual void controlBeginEdit(CControl* pControl)override { parentController->controlBeginEdit(pControl); }

	/** called when the user ends editing a control (mouse released) - we simply forward the call to the parent listener
	\param pControl - the control
	*/
	virtual void controlEndEdit(CControl* pControl)override { parentController->controlEndEdit(pControl); }

	/** register the control update receiver objects that
	    allow us to ultimately bind GUI controls to plugin variables (thread-safe of course)
	\param pControl - the control
	*/
	virtual void controlTagWillChange(CControl* pControl) override
	{
		pControl->setListener(parentController);
		parentController->controlTagWillChange(pControl);
		pControl->setListener(this);
	}

	/** register the control update receiver objects that
	allow us to ultimately bind GUI controls to plugin variables (thread-safe of course)
	\param pControl - the control
	*/
	virtual void controlTagDidChange(CControl* pControl) override
	{
		pControl->setListener(parentController);
		parentController->controlTagDidChange(pControl);
		pControl->setListener(this);
	}

protected:
	// --- the parent controller; we can issue IController commands to it!
	IController* parentController = nullptr; ///< pointer to owning listener

	// --- a CTextButton is the switcher (linkControl)
	CTextButton* linkControl = nullptr; ///< the link button is defined as a CTextButton (by me)

	// --- when linked, all of these controls move when one of them moves,
	//     regardless of their control tags
	typedef std::vector<CAnimKnob*> KnobList; ///< list of knobs
	KnobList linkedKnobs;

	// --- flag for linking knobs
	bool linkControls = false;		///< enable linking
};



}
