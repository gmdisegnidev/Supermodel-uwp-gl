#include "FilePicker.h"

#include <cctype>

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Storage::AccessCache;
using namespace Windows::UI::Core;

namespace
{
    std::string Narrow(Platform::String^ value)
    {
        if (value == nullptr)
            return "";

        std::string result;
        result.reserve(value->Length());
        for (unsigned int i = 0; i < value->Length(); ++i)
        {
            wchar_t ch = value->Data()[i];
            // Xbox ROM paths and Supermodel options are normally ASCII. Preserve
            // those exactly and avoid silently truncating other characters.
            result.push_back(ch <= 0x7f ? static_cast<char>(ch) : '?');
        }
        return result;
    }

    std::vector<std::string> SplitCommandLine(const std::string& command)
    {
        std::vector<std::string> result;
        std::string current;
        char quote = 0;

        for (size_t i = 0; i < command.size(); ++i)
        {
            char ch = command[i];
            if (quote != 0)
            {
                if (ch == quote)
                    quote = 0;
                else if (ch == '\\' && i + 1 < command.size() && command[i + 1] == quote)
                    current.push_back(command[++i]);
                else
                    current.push_back(ch);
            }
            else if (ch == '"' || ch == '\'')
            {
                quote = ch;
            }
            else if (std::isspace(static_cast<unsigned char>(ch)))
            {
                if (!current.empty())
                {
                    result.push_back(current);
                    current.clear();
                }
            }
            else
            {
                current.push_back(ch);
            }
        }

        if (!current.empty())
            result.push_back(current);
        return result;
    }

    void RememberFile(StorageFile^ file)
    {
        if (file != nullptr)
            StorageApplicationPermissions::FutureAccessList->AddOrReplace("ActivatedRomToken", file);
    }
}

template <typename T>
void WaitForAsync(IAsyncOperation<T>^ operation)
{
    while (operation->Status == AsyncStatus::Started)
        CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
}

std::vector<std::string> UWP::activation_arguments()
{
    std::vector<std::string> result;
    auto activation = AppInstance::GetActivatedEventArgs();
    if (activation == nullptr)
        return result;

    if (activation->Kind == ActivationKind::File)
    {
        auto file_args = dynamic_cast<FileActivatedEventArgs^>(activation);
        if (file_args != nullptr && file_args->Files->Size > 0)
        {
            auto file = dynamic_cast<StorageFile^>(file_args->Files->GetAt(0));
            if (file != nullptr)
            {
                RememberFile(file);
                result.push_back(Narrow(file->Path));
            }
        }
        return result;
    }

    if (activation->Kind != ActivationKind::Protocol)
        return result;

    auto protocol_args = dynamic_cast<ProtocolActivatedEventArgs^>(activation);
    if (protocol_args == nullptr || protocol_args->Uri == nullptr)
        return result;

    std::string rom;
    std::string command;
    auto decoder = ref new WwwFormUrlDecoder(protocol_args->Uri->Query);
    for (unsigned int i = 0; i < decoder->Size; ++i)
    {
        auto entry = decoder->GetAt(i);
        std::string name = Narrow(entry->Name);
        if (name == "rom" || name == "path" || name == "file")
            rom = Narrow(entry->Value);
        else if (name == "cmd")
            command = Narrow(entry->Value);
    }

    if (!command.empty())
        result = SplitCommandLine(command);
    else if (!rom.empty())
        result.push_back(rom);

    return result;
}

std::string UWP::pick_a_file()
{
    auto file_picker = ref new Windows::Storage::Pickers::FileOpenPicker();
    file_picker->SuggestedStartLocation = Windows::Storage::Pickers::PickerLocationId::ComputerFolder;
    file_picker->FileTypeFilter->Append(".zip");

    auto operation = file_picker->PickSingleFileAsync();
    WaitForAsync(operation);
    auto file = operation->GetResults();
    if (file == nullptr)
        return "";

    RememberFile(file);
    return Narrow(file->Path);
}
