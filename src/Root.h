#pragma once
#include "Root.g.h"

namespace winrt::XamlScratch::implementation
{
    struct Root : RootT<Root>
    {
        Root(Windows::UI::Xaml::Controls::Frame const&);

        //Event Handlers
        void ChangeTitleBarTheme(Windows::UI::Xaml::FrameworkElement const&, IInspectable const&);
        void ThemeChanged(IInspectable const&, Windows::UI::Xaml::Controls::SelectionChangedEventArgs const&);
        fire_and_forget LoadXamlRequested(IInspectable const&, Windows::UI::Xaml::RoutedEventArgs const&);
        void SetFontRequested(IInspectable const&, Windows::UI::Xaml::Controls::SelectionChangedEventArgs const&);
        void ChangeFontSizeRequested(Microsoft::UI::Xaml::Controls::NumberBox const&, Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs const&);
        void ModeChanged(IInspectable const&, Windows::UI::Xaml::RoutedEventArgs const&);
        void WheelChanged(IInspectable const&, Windows::UI::Xaml::Input::PointerRoutedEventArgs const&);
        fire_and_forget NavedTo(Microsoft::UI::Xaml::Controls::NavigationView const&, Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs const&);

        //Fix a strange bug on the ComboBox control inside the FlyoutPresenter control.
        //Don't remove/modify these function definitions!!!
        void HandleFlyoutClose(Windows::UI::Xaml::Controls::Primitives::FlyoutBase const&, Windows::UI::Xaml::Controls::Primitives::FlyoutBaseClosingEventArgs const&);
        void ListOpened(IInspectable const&, IInspectable const&);
        void ListClosed(IInspectable const&, IInspectable const&);

        //Destructor
        ~Root();

        //Static Methods        
        static winrt::XamlScratch::Root Init(Windows::UI::Xaml::Controls::Frame const& frame);
        
    private:
        Windows::UI::Xaml::Controls::Frame rootFrame{ nullptr };
        Windows::UI::Xaml::Controls::ContentDialog dialog;
        Windows::UI::ViewManagement::ApplicationViewTitleBar TitleBar = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView().TitleBar();
        event_token token; bool _ListOpened = false;

        Windows::Foundation::IAsyncAction LoadXaml();
        fire_and_forget ShowError(hresult_error const&);
        void TryIncreaseFontSize(int const& increment);

        void XamlTextChanged(IInspectable const&, Windows::UI::Xaml::Controls::TextChangedEventArgs const&);
    };
}

namespace winrt::XamlScratch::factory_implementation
{
    struct Root : RootT<Root, implementation::Root>
    {
    };
}