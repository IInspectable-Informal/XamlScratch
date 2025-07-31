#include "pch.h"
#include "App.h"
#include "Constants.h"

using ns winrt;
using ns Windows::ApplicationModel;
using ns Windows::ApplicationModel::Activation;
using ns Windows::ApplicationModel::Core;
using ns Windows::Foundation;
using ns Windows::UI;
using ns Windows::UI::Core;
using ns Windows::UI::ViewManagement;
using ns Windows::UI::Xaml;
using ns Windows::UI::Xaml::Controls;

namespace winrt::XamlScratch::implementation
{
    /// <summary>
    /// Creates the singleton application object.  This is the first line of authored code
    /// executed, and as such is the logical equivalent of main() or WinMain().
    /// </summary>
    App::App()
    {
        Suspending({ this, &App::OnSuspending });
        auto const& propSet = AppDC.Values();
        if (!propSet.HasKey(L"AppTheme"))
        { propSet.Insert(L"AppTheme", box_value(0)); }
        #if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
        UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e)
        {
            if (IsDebuggerPresent())
            {
                auto errorMessage = e.Message();
                __debugbreak();
            }
        });
        #endif
    }

    /// <summary>
    /// Invoked when the application is launched normally by the end user.  Other entry points
    /// will be used such as when the application is launched to open a specific file.
    /// </summary>
    /// <param name="e">Details about the launch request and process.</param>
    void App::OnLaunched(LaunchActivatedEventArgs const& e)
    {
        CoreApplication::GetCurrentView().TitleBar().ExtendViewIntoTitleBar(true);
        ApplicationView const& AppView = ApplicationView::GetForCurrentView();
        ApplicationViewTitleBar const& TitleBar = AppView.TitleBar();
        TitleBar.ButtonBackgroundColor(Colors::Transparent());
        TitleBar.ButtonInactiveBackgroundColor(Colors::Transparent());
        Frame rootFrame; Microsoft::UI::Xaml::Controls::BackdropMaterial::SetApplyToRootOrPageBackground(rootFrame, true);
        rootFrame.RequestedTheme(static_cast<ElementTheme>(unbox_value<int>(AppDC.Values().Lookup(L"AppTheme"))));
        rootFrame.Navigate(xaml_typename<Page>());
        rootFrame.Content().as<Page>().Content(local::Root::Init(rootFrame));
        auto curWin = Window::Current();
        curWin.Content(rootFrame);
        curWin.Activate();
    }

    /// <summary>
    /// Invoked when application execution is being suspended.  Application state is saved
    /// without knowing whether the application will be terminated or resumed with the contents
    /// of memory still intact.
    /// </summary>
    /// <param name="sender">The source of the suspend request.</param>
    /// <param name="e">Details about the suspend request.</param>
    void App::OnSuspending([[maybe_unused]] IInspectable const& sender, [[maybe_unused]] SuspendingEventArgs const& e)
    {
        // Save application state and stop any background activity
    }
}