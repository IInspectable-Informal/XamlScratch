#include "pch.h"
#include "Root.h"
#include "Root.g.cpp"
#include "Constants.h"

using ns winrt;
using ns Windows::ApplicationModel;
using ns Windows::Storage;
using ns Windows::System;
using ns Windows::UI;
using ns Windows::UI::Input;
using ns Windows::UI::Xaml;
using ns Windows::UI::Xaml::Controls;
using ns Windows::UI::Xaml::Controls::Primitives;
using ns Windows::UI::Xaml::Input;
using ns Windows::UI::Xaml::Markup;
using ns Windows::UI::Xaml::Media;
ns muxc = Microsoft::UI::Xaml::Controls;

namespace winrt::XamlScratch::implementation
{
    const event_token defaultToken = event_token();

    hstring ToHex(long const& hresult)
    {
        wchar_t buffer[11];
        swprintf_s(buffer, 11, L"0x%08X", static_cast<unsigned int>(hresult));
        return hstring(buffer);
    }

    //Event Handlers
    void Root::ChangeTitleBarTheme(FrameworkElement const& sender, IInspectable const&)
    {
        if (sender.ActualTheme() == ElementTheme::Light)
        { TitleBar.ButtonForegroundColor(Colors::Black()); TitleBar.ButtonInactiveForegroundColor(Colors::DarkGray()); }
        else { TitleBar.ButtonForegroundColor(Colors::White()); TitleBar.ButtonInactiveForegroundColor(Colors::LightGray()); }
        dialog.RequestedTheme(sender.ActualTheme());
    }

    void Root::ThemeChanged(IInspectable const&, SelectionChangedEventArgs const&)
    {
        int i = themeSwitcher().SelectedIndex();
        if (i >= 0)
        {
            auto themeIndex = (i + 1) % 3;
            rootFrame.RequestedTheme(static_cast<ElementTheme>(themeIndex));
            AppDC.Values().Insert(L"AppTheme", box_value(themeIndex));
        } else { themeSwitcher().SelectedIndex((unbox_value<int>(AppDC.Values().Lookup(L"AppTheme")) + 2) % 3); }
    }

    fire_and_forget Root::LoadXamlRequested(IInspectable const&, RoutedEventArgs const&)
    {
        LoadButton().IsEnabled(false);
        try
        {
            scratchPad().SelectedItem(prevNavItem());
            co_await LoadXaml();
        }
        catch (hresult_error const& ex)
        {
            scratchPad().SelectedItem(editorNavItem());
            ShowError(ex);
        } LoadButton().IsEnabled(true);
    }

    void Root::SetFontRequested(IInspectable const&, SelectionChangedEventArgs const&)
    {
        auto i0 = fontList().SelectedIndex();
        if (i0 > -1)
        {
            auto str = fontList().Items().GetAt(i0);
            XamlEditor().FontFamily(FontFamily(unbox_value<hstring>(str)));
            AppDC.Values().Insert(L"FontName", str);
        }
        else
        {
            uint32_t i = 0;
            fontList().Items().IndexOf(AppDC.Values().Lookup(L"FontName"), i);
            fontList().SelectedIndex(i);
        }
    }

    void Root::ChangeFontSizeRequested(muxc::NumberBox const&, muxc::NumberBoxValueChangedEventArgs const&)
    {
        double size = fontSizes().Value();
        if (!std::isnan(size))
        {
            XamlEditor().FontSize(size);
            AppDC.Values().Insert(L"FontSize", box_value(size));
        } else { fontSizes().Value(unbox_value<double>(AppDC.Values().Lookup(L"FontSize"))); }
    }

    void Root::ModeChanged(IInspectable const&, RoutedEventArgs const&)
    {
        bool isAutoLoadEnabled = AutoLoadSwitch().IsOn();
        LoadButton().Visibility(static_cast<::Visibility>(isAutoLoadEnabled));
        AppDC.Values().Insert(L"IsAutoLoadEnabled", box_value(isAutoLoadEnabled));
    }

    void Root::WheelChanged(IInspectable const& sender, PointerRoutedEventArgs const& e)
    {
        if (e.KeyModifiers() == VirtualKeyModifiers::Control)
        {
            int delta = e.GetCurrentPoint(sender.as<UIElement>()).Properties().MouseWheelDelta();
            e.Handled(delta);
            if (delta > 0)
            { TryIncreaseFontSize(1); }
            else if (delta < 0)
            { TryIncreaseFontSize(-1); }
        }
    }

    fire_and_forget Root::NavedTo(muxc::NavigationView const&, muxc::NavigationViewSelectionChangedEventArgs const&)
    {
        try
        {
            bool isEditorView = unbox_value<muxc::NavigationViewItem>(scratchPad().SelectedItem()) == editorNavItem();
            if (AutoLoadSwitch().IsOn() && !isEditorView)
            { co_await LoadXaml(); }
            XamlEditor().Visibility(isEditorView ? Visibility::Visible : Visibility::Collapsed);
            XamlPreview().Visibility(isEditorView ? Visibility::Collapsed : Visibility::Visible);
        }
        catch (hresult_error const& ex)
        {
            scratchPad().SelectedItem(editorNavItem());
            ShowError(ex);
        }
    }

    //Fix a strange bug on the ComboBox control inside the FlyoutPresenter control.
    //You can remove the code inside the function implementations to reproduce the bug.
    //⚠️If you removed these code, after you operated any control inside the FlyoutPresenter control,
    //which supports the text input function,
    //the Flyout will closed when you open the ComboBox selection list.
    //Patch code start
    void Root::HandleFlyoutClose(FlyoutBase const&, FlyoutBaseClosingEventArgs const& e)
    { if (_ListOpened) { e.Cancel(true); } }

    void Root::ListOpened(IInspectable const&, IInspectable const&)
    { _ListOpened = true; }

    void Root::ListClosed(IInspectable const&, IInspectable const&)
    { _ListOpened = false; }
    //Patch code end

    //Private Methods
    Windows::Foundation::IAsyncAction Root::LoadXaml()
    {
        BadgeAttention().Visibility(Visibility::Collapsed);
        if (token == defaultToken)
        { token = XamlEditor().TextChanged({ this, &Root::XamlTextChanged }); }
        co_await FileIO::WriteTextAsync(XamlFile, XamlEditor().Text());
        XamlPreview().Content(XamlReader::Load(L"<ContentPresenter xmlns=\"http://schemas.microsoft.com/winfx/2006/xaml/presentation\" xmlns:x=\"http://schemas.microsoft.com/winfx/2006/xaml\">" + XamlEditor().Text() + L"</ContentPresenter>"));
    }

    fire_and_forget Root::ShowError(hresult_error const& ex)
    {
        dialog.Title(box_value(L"Error"));
        dialog.Content(box_value(ex.message() + L"\nHRESULT: " + ToHex(ex.code())));
        dialog.CloseButtonText(L"Close");
        co_await dialog.ShowAsync();
    }

    void Root::TryIncreaseFontSize(int const& increment)
    {
        double newValue = fontSizes().Value() + increment;
        byte result = 0x0A * (newValue >= fontSizes().Minimum()) + (newValue <= fontSizes().Maximum());
        switch (result)
        {
            case 0x0B: //dec: 11, In Range
            { fontSizes().Value(newValue); break; }
            case 0x0A: //dec: 10, newValue > fontSizes().Maximum()
            { fontSizes().Value(fontSizes().Maximum()); break; }
            case 0x01: //dec: 01, newValue < fontSizes().Minimum()
            { fontSizes().Value(fontSizes().Minimum()); break; }
        }
    }

    void Root::XamlTextChanged(IInspectable const&, TextChangedEventArgs const&)
    {
        XamlEditor().TextChanged(token);
        token = event_token();
        BadgeAttention().Visibility(Visibility::Visible);
    }

    //Constructor
    Root::Root(Frame const& frame) : rootFrame(frame)
    {
        InitializeComponent();
        title().Text(Package::Current().DisplayName());
        themeSwitcher().SelectedIndex((unbox_value<int>(AppDC.Values().Lookup(L"AppTheme")) + 2) % 3);
        scratchPad().SelectedItem(editorNavItem());
        XamlEditor().Text(FileIO::ReadTextAsync(XamlFile).get());
        token = XamlEditor().TextChanged({ this, &Root::XamlTextChanged });
        dialog.Style(Application::Current().Resources().Lookup(box_value(L"DefaultContentDialogStyle")).as<::Style>());
        AutoLoadSwitch().IsOn(AppDC.Values().HasKey(L"IsAutoLoadEnabled") ? unbox_value<bool>(AppDC.Values().Lookup(L"IsAutoLoadEnabled")) : true);

        com_ptr<IDWriteFactory> pDWFactory;
        check_hresult(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)pDWFactory.put_void()));
        com_ptr<IDWriteFontCollection> pFontCollection;
        check_hresult(pDWFactory->GetSystemFontCollection(pFontCollection.put()));
        UINT32 fontCount = pFontCollection->GetFontFamilyCount();
        std::vector<IInspectable> fontFamilies;
        for (UINT32 i = 0; i < fontCount; ++i)
        {
            com_ptr<IDWriteFontFamily> pFontFamily;
            check_hresult(pFontCollection->GetFontFamily(i, pFontFamily.put()));
            com_ptr<IDWriteLocalizedStrings> pFamilyNames;
            check_hresult(pFontFamily->GetFamilyNames(pFamilyNames.put()));
            UINT32 index = 0;
            BOOL exists = FALSE;
            check_hresult(pFamilyNames->FindLocaleName(L"en-us", &index, &exists));
            if (exists)
            {
                UINT32 length = 0;
                check_hresult(pFamilyNames->GetStringLength(index, &length));
                std::wstring name(length + 1, L'\0');
                check_hresult(pFamilyNames->GetString(index, name.data(), length + 1));
                fontFamilies.emplace_back(box_value(name.c_str()));
            }
        } uint32_t i = 0; auto items = fontList().Items();
        items.ReplaceAll(fontFamilies);
        IInspectable str = AppDC.Values().HasKey(L"FontName") ? AppDC.Values().Lookup(L"FontName") : box_value(fontList().FontFamily().Source());
        items.IndexOf(str, i); fontList().SelectedIndex(i);
        fontSizes().Value(AppDC.Values().HasKey(L"FontSize") ? unbox_value<double>(AppDC.Values().Lookup(L"FontSize")) : 14);
    }

    //Destructor
    Root::~Root()
    {
        if (!(token == defaultToken))
        { XamlEditor().TextChanged(token); }
    }

    //Static Methods
    winrt::XamlScratch::Root Root::Init(Frame const& frame)
    {
        auto instance = make_self<implementation::Root>(frame);
        return instance.as<winrt::XamlScratch::Root>();
    }
}