#pragma once

namespace winrt
{
    const Windows::Storage::ApplicationDataContainer AppDC = Windows::Storage::ApplicationData::Current().LocalSettings();

    const Windows::Storage::StorageFile XamlFile = Windows::Storage::ApplicationData::Current().LocalFolder().CreateFileAsync(L"Xaml.xaml", Windows::Storage::CreationCollisionOption::OpenIfExists).get();
}