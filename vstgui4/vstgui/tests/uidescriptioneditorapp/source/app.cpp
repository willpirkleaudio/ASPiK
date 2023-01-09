// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "vstgui/lib/cbitmap.h"
#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/malloc.h"
#include "vstgui/lib/platform/common/genericoptionmenu.h"
#include "vstgui/lib/platform/iplatformframe.h"
#include "vstgui/standalone/include/helpers/appdelegate.h"
#include "vstgui/standalone/include/helpers/menubuilder.h"
#include "vstgui/standalone/include/helpers/uidesc/modelbinding.h"
#include "vstgui/standalone/include/helpers/value.h"
#include "vstgui/standalone/include/helpers/windowcontroller.h"
#include "vstgui/standalone/include/ialertbox.h"
#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/icommand.h"
#include "vstgui/uidescription/cstream.h"
#include "vstgui/uidescription/detail/uijsonpersistence.h"
#include "vstgui/uidescription/detail/uixmlpersistence.h"
#include "vstgui/uidescription/editing/uieditcontroller.h"
#include "vstgui/uidescription/editing/uieditmenucontroller.h"
#include "vstgui/uidescription/editing/uiundomanager.h"
#include "vstgui/uidescription/uicontentprovider.h"
#include "vstgui/uidescription/uidescription.h"
#include <chrono>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

using namespace Application;

#if VSTGUI_LIVE_EDITING
//------------------------------------------------------------------------
static void makeAndOpenWindow (bool darkTheme);

static constexpr IdStringPtr RunJSONXMLBenchmark = "Run JSON/XML Benchmark";
static const Command BenchmarkCommand {CommandGroup::File, RunJSONXMLBenchmark};

static constexpr IdStringPtr CommandNameLightTheme = "Light Theme";
static constexpr IdStringPtr CommandNameDarkTheme = "Dark Theme";
static constexpr IdStringPtr CommandCategoryTheme = "Theme";
static const Command CommandLightTheme {CommandCategoryTheme, CommandNameLightTheme};
static const Command CommandDarkTheme {CommandCategoryTheme, CommandNameDarkTheme};

//------------------------------------------------------------------------
class Controller : public WindowControllerAdapter, public ICommandHandler
{
public:
	Controller (bool darkTheme = false) : useDarkTheme (darkTheme)
	{
		IApplication::instance ().registerCommand (Commands::SaveDocument, 's');
		IApplication::instance ().registerCommand (Commands::RevertDocument, 0);
		IApplication::instance ().registerCommand (CommandLightTheme, 0);
		IApplication::instance ().registerCommand (CommandDarkTheme, 0);
#if VSTGUI_ENABLE_XML_PARSER
		IApplication::instance ().registerCommand (BenchmarkCommand, 0);
#endif

		std::string basePath = __FILE__;
		unixfyPath (basePath);
		removeLastPathComponent (basePath);
		removeLastPathComponent (basePath);
		removeLastPathComponent (basePath);
		removeLastPathComponent (basePath);
		basePath += "/uidescription/editing/";
		descPath = basePath + "uidescriptioneditor.uidesc";
		lightResPath = basePath + "uidescriptioneditor_res_light.uidesc";
		darkResPath = basePath + "uidescriptioneditor_res_dark.uidesc";
	}

	bool init ()
	{
		uidesc = makeOwned<UIDescription> (descPath.data ());
		if (!uidesc->parse ())
		{
			// TODO: show alert about error
			IApplication::instance ().quit ();
			return false;
		}
		uidesc->setFilePath (descPath.data ());
		lightResDesc = makeOwned<UIDescription> (lightResPath.data ());
		if (!lightResDesc->parse ())
		{
			// TODO: show alert about error
			IApplication::instance ().quit ();
			return false;
		}
		darkResDesc = makeOwned<UIDescription> (darkResPath.data ());
		if (!darkResDesc->parse ())
		{
			darkResDesc = nullptr;
		}

		if (useDarkTheme && darkResDesc)
		{
			uidesc->setSharedResources (darkResDesc);
		}
		else
		{
			uidesc->setSharedResources (lightResDesc);
		}
		editController = makeOwned<UIEditController> (uidesc);

		return true;
	}

	bool save ()
	{
		if (uidesc->save (descPath.data (), UIDescription::kWriteImagesIntoUIDescFile))
		{
			if (useDarkTheme && darkResDesc)
			{
				darkResDesc->save (darkResPath.data (), UIDescription::kWriteImagesIntoUIDescFile);
			}
			else if (lightResDesc)
			{
				lightResDesc->save (lightResPath.data (),
									UIDescription::kWriteImagesIntoUIDescFile);
			}
			return true;
		}
		return false;
	}

	void beforeShow (IWindow& window) override
	{
		win = &window;
		CRect r;
		r.setSize (window.getSize ());
		auto frame = makeOwned<CFrame> (r, nullptr);
		frame->enableTooltips (true);
		editController->setDarkTheme (useDarkTheme);
		if (auto view = editController->createEditView ())
		{
			editController->remember (); // view will forget it too
			frame->setViewSize (view->getViewSize ());
			frame->addView (view);
			window.setContentView (frame);

			auto focusDrawingSettings = uidesc->getFocusDrawingSettings ();
			if (focusDrawingSettings.enabled)
			{
				CColor focusColor;
				if (uidesc->getColor (focusDrawingSettings.colorName, focusColor))
					frame->setFocusColor (focusColor);
				frame->setFocusWidth (focusDrawingSettings.width);
				frame->setFocusDrawingEnabled (true);
			}
		}
	}

	enum class SaveChanges
	{
		Save,
		Cancel,
		DontSave,
	};

	SaveChanges askSaveChanges ()
	{
		if (win && !editController->getUndoManager ()->isSavePosition ())
		{
			AlertBoxConfig config;
			config.headline = "There are unsaved changes.";
			config.description = "Do you want to save the changes ?";
			config.defaultButton = "Save";
			config.secondButton = "Cancel";
			config.thirdButton = "Don't Save";
			auto result = IApplication::instance ().showAlertBox (config);
			switch (result)
			{
				case AlertResult::DefaultButton:
				{
					save ();
					return SaveChanges::Save;
				}
				case AlertResult::SecondButton: return SaveChanges::Cancel;
				default: break;
			}
		}
		return SaveChanges::DontSave;
	}

	bool checkSaveChanges ()
	{
		switch (askSaveChanges ())
		{
			case SaveChanges::Save:
			{
				save ();
				return true;
			}
			case SaveChanges::Cancel: return false;
		}
		return true;
	}

	bool canClose (const IWindow& window) override
	{
		return checkSaveChanges ();
	}

	void onClosed (const IWindow& window) override
	{
		if (IApplication::instance ().getWindows ().empty ())
			IApplication::instance ().quit ();
	}

	bool canHandleCommand (const Command& command) override
	{
		if (command == Commands::SaveDocument)
			return true;
		if (command == Commands::Undo)
			return editController->getUndoManager ()->canUndo ();
		if (command == Commands::Redo)
			return editController->getUndoManager ()->canRedo ();
		if (command == Commands::RevertDocument)
			return !editController->getUndoManager ()->isSavePosition ();
		if (command == CommandLightTheme && lightResDesc)
			return uidesc->getSharedResources () != lightResDesc;
		if (command == CommandDarkTheme && darkResDesc)
			return uidesc->getSharedResources () != darkResDesc;
		if (command == BenchmarkCommand)
			return true;
		return false;
	}

	void reopenWindow ()
	{
		makeAndOpenWindow (useDarkTheme);
		if (auto window = win)
		{
			win = nullptr;
			window->close ();
		}
	}

	bool handleCommand (const Command& command) override
	{
		if (command == Commands::SaveDocument)
			return save ();
		if (command == Commands::Undo)
			return editController->getMenuController ()->handleCommand ("Edit", "Undo");
		if (command == Commands::Redo)
			return editController->getMenuController ()->handleCommand ("Edit", "Redo");
		if (command == Commands::RevertDocument)
		{
			reopenWindow ();
			return true;
		}
		if (command == CommandLightTheme)
		{
			if (!checkSaveChanges ())
				return true;
			useDarkTheme = false;
			reopenWindow ();
			return true;
		}
		if (command == CommandDarkTheme)
		{
			if (!checkSaveChanges ())
				return true;
			useDarkTheme = true;
			reopenWindow ();
			return true;
		}
		if (command == BenchmarkCommand)
		{
			runBenchmark ();
			return true;
		}
		return false;
	}

	void onSetContentView (IWindow& window, const SharedPointer<CFrame>& contentView) override
	{
		if (auto platformFrame = contentView->getPlatformFrame ())
		{
			platformFrame->setupGenericOptionMenu (true);
		}
	}

	void runBenchmark ()
	{
#if VSTGUI_ENABLE_XML_PARSER
		static constexpr auto iterations = 100u;

		if (!uidesc)
			return;
		auto node = uidesc->getRootNode ();
		if (!node)
			return;

		CMemoryStream xmlData (1024, 1024, false);
		CMemoryStream jsonData (1024, 1024, false);

		Detail::UIXMLDescWriter xmlWriter;
		if (!xmlWriter.write (xmlData, node))
			return;
		xmlData.rewind ();

		if (!Detail::UIJsonDescWriter::write (jsonData, node))
			return;
		jsonData.end ();
		jsonData.rewind ();

		InputStreamContentProvider xmlContentProvider (xmlData);

		auto xmlStartTime = std::chrono::high_resolution_clock::now ();
		for (auto i = 0u; i < iterations; ++i)
		{
			xmlData.rewind ();
			Detail::UIXMLParser parser;
			if (!parser.parse (&xmlContentProvider))
				return;
		}
		auto xmlEndTime = std::chrono::high_resolution_clock::now ();

		InputStreamContentProvider jsonContentProvider (jsonData);

		auto jsonStartTime = std::chrono::high_resolution_clock::now ();
		for (auto i = 0u; i < iterations; ++i)
		{
			jsonData.rewind ();
			if (!Detail::UIJsonDescReader::read (jsonContentProvider))
				return;
		}
		auto jsonEndTime = std::chrono::high_resolution_clock::now ();

		auto xmlDuration =
		    std::chrono::duration_cast<std::chrono::milliseconds> (xmlEndTime - xmlStartTime)
		        .count ();
		auto jsonDuration =
		    std::chrono::duration_cast<std::chrono::milliseconds> (jsonEndTime - jsonStartTime)
		        .count ();

		printf ("xml :%lld\n", xmlDuration);
		printf ("json:%lld\n", jsonDuration);
#endif
	}

	std::string descPath;
	std::string lightResPath;
	std::string darkResPath;
	SharedPointer<UIDescription> uidesc;
	SharedPointer<UIDescription> lightResDesc;
	SharedPointer<UIDescription> darkResDesc;
	SharedPointer<UIEditController> editController;
	IWindow* win {nullptr};
	bool useDarkTheme {false};
};

//------------------------------------------------------------------------
void makeAndOpenWindow (bool darkTheme)
{
	auto controller = std::make_shared<Controller> (darkTheme);
	if (controller->init ())
	{
		WindowConfiguration config;
		config.style.border ().size ().close ().centered ();
		config.title = "UIDescriptionEditor";
		config.autoSaveFrameName = "UIDescriptionEditorWindowFrame";
		config.size = {500, 500};
		if (auto window = IApplication::instance ().createWindow (config, controller))
			window->show ();
		else
			IApplication::instance ().quit ();
	}
}
#endif

//------------------------------------------------------------------------
class UIDescriptionEditorApp : public DelegateAdapter, public MenuBuilderAdapter
{
public:
	UIDescriptionEditorApp ()
	: DelegateAdapter ({"UIDescriptionEditorApp", "1.0.0", "vstgui.uidescriptioneditorapp"})
	{
	}

	bool prependMenuSeparator (const Interface& context, const Command& cmd) const override
	{
		if (cmd == Commands::CloseWindow || cmd == BenchmarkCommand)
			return true;
		return false;
	}

	SortFunction getCommandGroupSortFunction (const Interface& context,
	                                          const UTF8String& group) const override
	{
		if (group == CommandGroup::File)
		{
			return [] (const UTF8String& lhs, const UTF8String& rhs) {
				static const auto order = {
				    Commands::NewDocument.name,    Commands::OpenDocument.name,
				    Commands::SaveDocument.name,   Commands::SaveDocumentAs.name,
				    Commands::RevertDocument.name, BenchmarkCommand.name,
				    Commands::CloseWindow.name};
				auto leftIndex = std::find (order.begin (), order.end (), lhs);
				auto rightIndex = std::find (order.begin (), order.end (), rhs);
				return std::distance (leftIndex, rightIndex) > 0;
			};
		}
		return {};
	}

	void finishLaunching () override
	{
#if VSTGUI_LIVE_EDITING
		makeAndOpenWindow (false);
#else
		IApplication::instance ().quit ();
		AlertBoxConfig config;
		config.headline = "UIDescriptionEditorApp only works in Debug mode";
		IApplication::instance ().showAlertBox (config);
#endif
	}
};

static Init gAppDelegate (std::make_unique<UIDescriptionEditorApp> ());

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
