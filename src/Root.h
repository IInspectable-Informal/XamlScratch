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
        fire_and_forget NavedTo(Microsoft::UI::Xaml::Controls::NavigationView const&, Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs const&);

        //Static Methods        
        static winrt::XamlScratch::Root Init(Windows::UI::Xaml::Controls::Frame const& frame);
        
    private:
        Windows::UI::Xaml::Controls::Frame rootFrame{ nullptr };
        Windows::UI::Xaml::Controls::ContentDialog dialog;
        Windows::UI::ViewManagement::ApplicationViewTitleBar TitleBar = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView().TitleBar();

        fire_and_forget ShowError(hresult_error const&);
    };
}

namespace winrt::XamlScratch::factory_implementation
{
    struct Root : RootT<Root, implementation::Root>
    {
    };
}