#include "pch.h"
#include "Root.h"
#include "Root.g.cpp"
#include "Constants.h"
#include <dwrite.h>
#include <winrt/Windows.UI.Popups.h>

using ns winrt;
using ns Windows::ApplicationModel;
using ns Windows::Storage;
using ns Windows::UI;
using ns Windows::UI::Xaml;
using ns Windows::UI::Xaml::Controls;
using ns Windows::UI::Xaml::Markup;
ns muxc = Microsoft::UI::Xaml::Controls;

namespace winrt::XamlScratch::implementation
{
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

    fire_and_forget Root::NavedTo(muxc::NavigationView const&, muxc::NavigationViewSelectionChangedEventArgs const&)
    {
        try
        {
            bool isEditorView = unbox_value<muxc::NavigationViewItem>(scratchPad().SelectedItem()) == editorNavItem();
            if (!isEditorView)
            {
                co_await FileIO::WriteTextAsync(XamlFile, XamlEditor().Text());
                XamlPreview().Content(XamlReader::Load(L"<ContentPresenter xmlns=\"http://schemas.microsoft.com/winfx/2006/xaml/presentation\" xmlns:x=\"http://schemas.microsoft.com/winfx/2006/xaml\">" + XamlEditor().Text() + L"</ContentPresenter>"));
            }
            XamlEditor().Visibility(isEditorView ? Visibility::Visible : Visibility::Collapsed);
            XamlPreview().Visibility(isEditorView ? Visibility::Collapsed : Visibility::Visible);
        }
        catch ([[maybe_unused]] hresult_error const& ex)
        {
            scratchPad().SelectedItem(editorNavItem());
            ShowError(ex);
        }
    }

    //Private Methods
    fire_and_forget Root::ShowError(hresult_error const& ex)
    {
        dialog.Title(box_value(L"Error"));
        dialog.Content(box_value(ex.message() + L"\nHRESULT: " + ToHex(ex.code())));
        dialog.CloseButtonText(L"Close");
        co_await dialog.ShowAsync();
    }

    //Constructor
    Root::Root(Frame const& frame) : rootFrame(frame)
    {
        InitializeComponent();
        title().Text(Package::Current().DisplayName());
        themeSwitcher().SelectedIndex((unbox_value<int>(AppDC.Values().Lookup(L"AppTheme")) + 2) % 3);
        scratchPad().SelectedItem(editorNavItem());
        XamlEditor().Text(FileIO::ReadTextAsync(XamlFile).get());
        dialog.Style(Application::Current().Resources().Lookup(box_value(L"DefaultContentDialogStyle")).as<::Style>());

        com_ptr<IDWriteFactory> pDWFactory;
        check_hresult(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)pDWFactory.put_void()));
        com_ptr<IDWriteFontCollection> pFontCollection;
        check_hresult(pDWFactory->GetSystemFontCollection(pFontCollection.put()));
        UINT32 fontCount = pFontCollection->GetFontFamilyCount();
        std::vector<hstring> fontFamilies;
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
                fontFamilies.emplace_back(name.c_str());
            }
        } uint32_t ic = fontFamilies.size(); auto items = fontList().Items(); auto defaultFont = fontList().FontFamily().Source();
        for (uint32_t i = 0; i < ic; ++i)
        {
            items.Append(box_value(fontFamilies[i]));
            if (defaultFont == fontFamilies[i])
            { fontList().SelectedIndex(i); }
        }
        Windows::UI::Popups::MessageDialog(defaultFont).ShowAsync();
    }

    //Static Methods
    winrt::XamlScratch::Root Root::Init(Frame const& frame)
    {
        auto instance = make_self<implementation::Root>(frame);
        return instance.as<winrt::XamlScratch::Root>();
    }
}