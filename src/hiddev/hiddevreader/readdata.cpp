#include "hiddev/hiddevreader.h"
#include "log/log.h"

using namespace kmicki::log;

namespace kmicki::hiddev
{
    // Definition - ReadData
    HidDevReader::ReadData::ReadData(std::string const& _inputFilePath, int const& _frameLen)
    : inputFilePath(_inputFilePath), inputStream(), startMarker(0),
      Data(new std::vector<char>(HidDevReader::cInputRecordLen*_frameLen),
           new std::vector<char>(HidDevReader::cInputRecordLen*_frameLen), 
           new std::vector<char>(HidDevReader::cInputRecordLen*_frameLen)),
      Unsynced()
    { }

    HidDevReader::ReadData::~ReadData()
    { }

    void HidDevReader::ReadData::ReconnectInput()
    {
        DisconnectInput();
        Log("HidDevReader::ReadData: Opening hiddev file.",LogLevelDebug);
        inputStream.open(inputFilePath,std::ios_base::binary);
    }

    void HidDevReader::ReadData::DisconnectInput()
    {
        if(inputStream.is_open())
        {
            Log("HidDevReader::ReadData: Closing hiddev file.",LogLevelDebug);
            inputStream.close();
            inputStream.clear();
        }
    }

    uint32_t const& ExtractFirst4Bytes(std::vector<char> const& data)
    {
        return *(reinterpret_cast<uint32_t const*>(data.data()));
    }
 
    void HidDevReader::ReadData::Execute()
    {
        static const int cReportMissedTicksPeriod = 250;
        int missedTicks = 0;
        int nonMissedTicks = 0;

        ReconnectInput();
        if(inputStream.fail())
            throw std::runtime_error("HidDevReader::ReadData: Problem opening hiddev file. Are priviliges granted?");
        auto const& data = Data.GetPointerToFill();

        Log("HidDevReader::ReadData: Started.",LogLevelDebug);
        
        while(ShouldContinue())
        {
            //tick.WaitForSignal();

            if(!ShouldContinue())
                break;

            inputStream.read(data->data(),data->size());

            if(!CheckData(data))
                continue;

            HandleMissedTicks("HidDevReader::ReadData","HID frames",Data.WasReceived(),missedTicks,cReportMissedTicksPeriod,nonMissedTicks);

            Data.SendData();
        }

        DisconnectInput();
        Log("HidDevReader::ReadData: Stopped.",LogLevelDebug);
    }

    void HidDevReader::ReadData::FlushPipes()
    {
        //tick.SendSignal();
    }

    bool HidDevReader::ReadData::CheckData(std::unique_ptr<std::vector<char>> const& data)
    {
        static const uint32_t cFirst4Bytes = 0xFFFF0002;
        static const uint32_t cFirst4BytesAlternative = 0xFFFF0001;

        bool inputFail = inputStream.fail();
        bool startMarkerFail = false;

        if(!inputFail)
        {
            startMarkerFail = ExtractFirst4Bytes(*data) != cFirst4Bytes;
            if(startMarkerFail && startMarker.size() > 0 && ExtractFirst4Bytes(*data) == cFirst4BytesAlternative)
            {
                startMarkerFail = false;
                // Check special start marker
                for(int i = cByteposInput, j=0;j<startMarker.size();++j,i+=cInputRecordLen)
                    if(startMarker[j] != (*data)[i])
                    {
                        startMarkerFail = true;
                        break;
                    }
            }
        }

        if(inputFail || startMarkerFail)
        {
            if(inputFail)
                Log("HidDevReader::ReadData: Reading from hiddev file failed.",LogLevelDebug);
            else
                Log("HidDevReader::ReadData: Reading from hiddev file started in the middle of the HID frame.",LogLevelDebug);
            // Failed to read a frame
            // or start in the middle of the input frame
            ReconnectInput();
            Unsynced.SendSignal();
            return false;
        }
        return true;
    }

    void HidDevReader::ReadData::SetStartMarker(std::vector<char> const& marker)
    {
        startMarker = marker;
    }
}