#pragma once

#include "fmt/format.h"
#include "rapidjson/encodings.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/document.h"

#include "CommonStructs.h"

#include <codecvt>
#include <vector>

enum ConnectType
{
	Connect_Client = 0x01,
 
	Connect_PhotoViewer,
	Connect_PDFViewer,
	Connect_OfficeViewer,
	Connect_UnknowFileViewer,
	Connect_Cast,
	Connect_VideoStream,
	Connect_NativeSystem,
	Connect_WebViewer,
	Connect_NetStream,
	Connect_CADViewer,
	Connect_DesktopCapture
};
 
namespace JsonCommand
{  
	// turn unicode string to json str, like ("\u0418\u043d")
	static std::string UnicodeToASCII(const wchar_t* source) {
		// convert wchar to utf8   
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter; 

		using namespace rapidjson;
		StringBuffer s;
		Writer<StringBuffer, Document::EncodingType, ASCII<> > writer(s);

		//std::string txt = u8"G:\\TestMedia\\��ɫ.png"; 
		writer.String(converter.to_bytes(source).c_str());
		return s.GetString();
	}

	// turn utf8 string to json str, like ("\u0418\u043d")
	static std::string Utf8ToASCII(const char* source) { 

		using namespace rapidjson;
		StringBuffer s;
		Writer<StringBuffer, Document::EncodingType, ASCII<> > writer(s);
		 
		writer.String(source);
		return s.GetString();
	}

	static std::string UnicodeToUtf8(const std::wstring& wstr)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convert;
		return convert.to_bytes(wstr);
	}

	static std::wstring Utf8ToUnicode(const char* utf8) {

		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convert;
		return convert.from_bytes(utf8); 
	}

	static std::string GuidToString(const GUID& guid)
	{
		char buf[64] = { 0 };
		sprintf_s(buf, sizeof(buf), "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
			guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
		return std::string(buf);
	} 

	enum CommandType
	{
		CMD_Idle = 0x1,
		CMD_SyncTime,


		CMD_Connect,
		CMD_Disconnect,
		CMD_CloseService,
		CMD_ClientUpdate,
		CMD_FrameUpdate,
		CMD_ChangeSize,
		CMD_MuteAudio,
		CMD_TouchGesture,
		CMD_RequestKeyBoard,
		CMD_KeyBoardInput,
		CMD_RdpRefreshSize,
		CMD_KeyAction,
		CMD_TouchGestureEx
	};

	enum ClientUpdateType
	{
		Client_Add = 1,
		Client_Update,
		Client_Remove, 
	};
 


	static std::string SyncTime(int64_t syncTime) {
		return fmt::format(R"( {{"action":{}, "time":{} }} )", CMD_SyncTime, syncTime);
	}

	//////////////////common commands //////////////////////////////////////////////
#pragma region common commands	

	static std::string Connect(int type, const char* clientId = NULL, const char* Instance = NULL) {
		return fmt::format(R"( {{"action":{}, "pid":{}, "type":{}, "client_id":"{}", "instance":"{}" }} )",
			CMD_Connect, ::GetCurrentProcessId(), type, clientId ? clientId : "", Instance ? Instance : "");
	}
	 
	static std::string CloseService() {
		return fmt::format(R"( {{"action":{}, "pid":{} }} )", 
			CMD_CloseService, ::GetCurrentProcessId());
	}


	static std::string ClientUpdate() {
		return fmt::format(R"( {{"action":{}, "pid":{} }} )", 
			CMD_ClientUpdate, ::GetCurrentProcessId());
	}

	static std::string FrameUpdate(const char* clientId, const char* Instance, HANDLE hFrame) {
		return fmt::format(R"( {{"action":{}, "client_id":"{}", "instance":"{}", "frame_handle":{} }} )",
			CMD_FrameUpdate, clientId, Instance, (INT64)hFrame);
	}

	static std::string ChangeSize(const char* clientId, const char* Instance, int width, int height) {
		return fmt::format(R"( {{"action":{}, "client_id":"{}", "instance":"{}", "width":{}, "height":{} }} )",
			CMD_ChangeSize, clientId, Instance, width, height);
	}

	static std::string MuteAudio(const char* clientId, const char* Instance, int mute) {
		return fmt::format(R"( {{"action":{}, "client_id":"{}", "instance":"{}", "mute":{} }} )",
			CMD_MuteAudio, clientId, Instance? Instance:"", mute);
	}

#pragma endregion

};

