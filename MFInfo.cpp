#include "MFInfo.h"

#include <mfapi.h>
#include <mfreadwrite.h>
#include <Mferror.h>
#include <atlbase.h>
#include <Ks.h>
#include <Codecapi.h>

#include <sstream>

#ifndef CODECAPI_AVEncH264CABACEnable
GUID CODECAPI_AVEncH264CABACEnable = {0xee6cad62, 0xd305, 0x4248, 0xa5, 0xe, 0xe1, 0xb2, 0x55, 0xf7, 0xca, 0xf8};
#endif

namespace mfinfo {

MediaInfo::MediaInfo() : _videolist(), _audiolist()
{
    // initialize COM
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	// Start up Media Foundation platform.
    MFStartup(MF_VERSION);
}

void MediaInfo::clear()
{
	_videolist.clear();
	_audiolist.clear();
}

std::string BytesToSize( long Bytes )        
{
    float tb = 1099511627776;
    float gb = 1073741824;
    float mb = 1048576;
    float kb = 1024;

    char returnSize[256];

    if( Bytes >= tb )
        sprintf_s(returnSize, "%.2f TB", (float)Bytes/tb);        
    else if( Bytes >= gb && Bytes < tb )
        sprintf_s(returnSize, "%.2f GB", (float)Bytes/gb);
    else if( Bytes >= mb && Bytes < gb )
        sprintf_s(returnSize, "%.2f MB", (float)Bytes/mb);   
    else if( Bytes >= kb && Bytes < mb )
        sprintf_s(returnSize, "%.2f KB", (float)Bytes/kb);
    else if ( Bytes < kb)
        sprintf_s(returnSize, "%.2f Bytes", Bytes);
    else
        sprintf_s(returnSize, "%.2f Bytes", Bytes);

	return std::string(returnSize);
}

std::string BytesToSizePerSec( long Bytes )        
{
	std::string r(BytesToSize(Bytes));
	r.append("/s");
	return r;
}

void loadMediaType_Video(MediaInfo &info, int index, CComPtr<IMFMediaType> pType)
{
	VideoInfo::Ptr vinfo(new VideoInfo);
	vinfo->index = index;

	std::stringstream vdesc;

	// SUBTYPE
	GUID subtype;
	checkHR(pType->GetGUID(MF_MT_SUBTYPE, &subtype), "load subtype");
	vinfo->videoFormat = subtype;

	if (IsEqualGUID(subtype, MFVideoFormat_H264)) vinfo->videoFormatStr = "H.264";
	else if (IsEqualGUID(subtype, MFVideoFormat_MP43)) vinfo->videoFormatStr = "MPEG-4";
	else if (IsEqualGUID(subtype, MFVideoFormat_MP4S)) vinfo->videoFormatStr = "MPEG-4";
	else if (IsEqualGUID(subtype, MFVideoFormat_M4S2)) vinfo->videoFormatStr = "MPEG-4 part 2";
	else if (IsEqualGUID(subtype, MFVideoFormat_MP4V)) vinfo->videoFormatStr = "MPEG-4 part 2";
	else if (IsEqualGUID(subtype, MFVideoFormat_WMV1)) vinfo->videoFormatStr = "Windows Media Video 7";
	else if (IsEqualGUID(subtype, MFVideoFormat_WMV2)) vinfo->videoFormatStr = "Windows Media Video 8";
	else if (IsEqualGUID(subtype, MFVideoFormat_WMV3)) vinfo->videoFormatStr = "Windows Media Video 9";
	else if (IsEqualGUID(subtype, MFVideoFormat_WVC1)) vinfo->videoFormatStr = "WMV VC-1";
	else if (IsEqualGUID(subtype, MFVideoFormat_MPG1)) vinfo->videoFormatStr = "MPEG-1";
	else if (IsEqualGUID(subtype, MFVideoFormat_MPG2)) vinfo->videoFormatStr = "MPEG-2";
	else if (IsEqualGUID(subtype, MFVideoFormat_MSS1)) vinfo->videoFormatStr = "Windows Media Video 7 (Screen)";
	else if (IsEqualGUID(subtype, MFVideoFormat_MSS2)) vinfo->videoFormatStr = "Windows Media Video 9 (Screen)";

	UINT32 v1, v2;

	// FRAME SIZE
	checkHR(MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &v1, &v2), "get frame size");
	vinfo->width = v1;
	vinfo->height = v2;

	// FRAME RATE
	checkHR(MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, &v1, &v2), "get frame rate");
	if (v2 > 0)
		vinfo->frameRate = (double)v1 / (double)v2;

	// BIT RATE
	HRESULT hr = pType->GetUINT32(MF_MT_AVG_BITRATE, &v1);
	if (hr != MF_E_ATTRIBUTENOTFOUND)
	{
		checkHR(hr, "get bit rate");
		vinfo->bitRate = v1;
		vinfo->bitRateStr = BytesToSizePerSec(v1);
	}

	// CABAC
	/*
	VARIANT_BOOL vbool;
	hr = pType->GetUINT32(CODECAPI_AVEncH264CABACEnable, &v1);
	if (hr != MF_E_ATTRIBUTENOTFOUND)
	{
		checkHR(hr, "get CABAC");
	}
	*/

	// build description
	vdesc << vinfo->videoFormatStr << " " << vinfo->width << "x" << vinfo->height;
	if (vinfo->bitRate >= 0)
		vdesc << " [" << vinfo->bitRateStr << "]";

	// H.264 properties
	if (IsEqualGUID(subtype, MFVideoFormat_H264))
	{
		vinfo->info_h264.reset(new VideoInfo_H264);

		std::stringstream vinfodesc;

		// MPEG2 PROFILE
		hr = pType->GetUINT32(MF_MT_MPEG2_PROFILE, &v1);
		if (hr != MF_E_ATTRIBUTENOTFOUND)
		{
			checkHR(hr, "get H.264 profile");

			vinfo->info_h264->profile = v1;

			if (v1 == eAVEncH264VProfile_Simple) vinfo->info_h264->profileStr = "Simple";
			else if (v1 == eAVEncH264VProfile_Base) vinfo->info_h264->profileStr = "Base";
			else if (v1 == eAVEncH264VProfile_Main) vinfo->info_h264->profileStr = "Main";
			else if (v1 == eAVEncH264VProfile_High) vinfo->info_h264->profileStr = "High";
			else if (v1 == eAVEncH264VProfile_422) vinfo->info_h264->profileStr = "High 4:2:2";
			else if (v1 == eAVEncH264VProfile_High10) vinfo->info_h264->profileStr = "High 10";
			else if (v1 == eAVEncH264VProfile_444) vinfo->info_h264->profileStr = "High 4:4:4";
			else if (v1 == eAVEncH264VProfile_Extended) vinfo->info_h264->profileStr = "Extended";

			vinfodesc << "Profile " << vinfo->info_h264->profileStr << "@";
		}

		// MPEG2 LEVEL
		hr = pType->GetUINT32(MF_MT_MPEG2_LEVEL, &v1);
		if (hr != MF_E_ATTRIBUTENOTFOUND)
		{
			checkHR(hr, "get H.264 level");

			vinfo->info_h264->level = v1;

			if (v1 == eAVEncH264VLevel1) vinfo->info_h264->levelStr = "1";
			else if (v1 == eAVEncH264VLevel1_b) vinfo->info_h264->levelStr = "1b";
			else if (v1 == eAVEncH264VLevel1_1) vinfo->info_h264->levelStr = "1.1";
			else if (v1 == eAVEncH264VLevel1_2) vinfo->info_h264->levelStr = "1.2";
			else if (v1 == eAVEncH264VLevel1_3) vinfo->info_h264->levelStr = "1.3";

			else if (v1 == eAVEncH264VLevel2) vinfo->info_h264->levelStr = "2";
			else if (v1 == eAVEncH264VLevel2_1) vinfo->info_h264->levelStr = "2.1";
			else if (v1 == eAVEncH264VLevel2_2) vinfo->info_h264->levelStr = "2.2";

			else if (v1 == eAVEncH264VLevel3) vinfo->info_h264->levelStr = "3";
			else if (v1 == eAVEncH264VLevel3_1) vinfo->info_h264->levelStr = "3.1";
			else if (v1 == eAVEncH264VLevel3_2) vinfo->info_h264->levelStr = "3.2";

			else if (v1 == eAVEncH264VLevel4) vinfo->info_h264->levelStr = "4";
			else if (v1 == eAVEncH264VLevel4_1) vinfo->info_h264->levelStr = "4.1";
			else if (v1 == eAVEncH264VLevel4_2) vinfo->info_h264->levelStr = "4.2";

			else if (v1 == eAVEncH264VLevel5) vinfo->info_h264->levelStr = "5";
			else if (v1 == eAVEncH264VLevel5_1) vinfo->info_h264->levelStr = "5.1";
			else if (v1 == 52/*eAVEncH264VLevel5_2*/) vinfo->info_h264->levelStr = "5.2";

			vinfodesc << vinfo->info_h264->levelStr;
		}

		vinfo->info_h264->description = vinfodesc.str();

		vdesc << " (" << vinfodesc.str() << ")";
	}

	vinfo->description = vdesc.str();

	// add to info
	info.videoList().push_back(vinfo);
}

void loadMediaType_Audio(MediaInfo &info, int index, CComPtr<IMFMediaType> pType)
{
	AudioInfo::Ptr ainfo(new AudioInfo);
	ainfo->index = index;

	std::stringstream adesc;

	// SUBTYPE
	GUID subtype;
	checkHR(pType->GetGUID(MF_MT_SUBTYPE, &subtype), "load subtype");
	ainfo->audioFormat = subtype;

	if (IsEqualGUID(subtype, MFAudioFormat_MP3)) ainfo->audioFormatStr = "MP3";
	else if (IsEqualGUID(subtype, MFAudioFormat_PCM)) ainfo->audioFormatStr = "PCM";
	else if (IsEqualGUID(subtype, MFAudioFormat_DTS)) ainfo->audioFormatStr = "DST";
	else if (IsEqualGUID(subtype, MFAudioFormat_Dolby_AC3_SPDIF)) ainfo->audioFormatStr = "Dolby AC-3 SPDIF";
	else if (IsEqualGUID(subtype, MFAudioFormat_DRM)) ainfo->audioFormatStr = "DRM";
	else if (IsEqualGUID(subtype, MFAudioFormat_WMAudioV8)) ainfo->audioFormatStr = "Windows Media Audio 8";
	else if (IsEqualGUID(subtype, MFAudioFormat_WMAudioV9)) ainfo->audioFormatStr = "Windows Media Audio 9";
	else if (IsEqualGUID(subtype, MFAudioFormat_WMAudio_Lossless)) ainfo->audioFormatStr = "Windows Media Audio Lossless";
	else if (IsEqualGUID(subtype, MFAudioFormat_MPEG)) ainfo->audioFormatStr = "MPEG";
	else if (IsEqualGUID(subtype, MFAudioFormat_AAC)) ainfo->audioFormatStr = "AAC";
	else if (IsEqualGUID(subtype, MFAudioFormat_ADTS)) ainfo->audioFormatStr = "ADTS";

	UINT32 v1;

	// CHANNELS
	HRESULT hr = pType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &v1);
	if (hr != MF_E_ATTRIBUTENOTFOUND)
	{
		checkHR(hr, "get channels");
		ainfo->channels = v1;
	}

	// SAMPLES PER SECOND
	hr = pType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &v1);
	if (hr != MF_E_ATTRIBUTENOTFOUND)
	{
		checkHR(hr, "get samples per second");
		ainfo->samplesPerSecond = v1;
	}

	// BITS PER SAMPLE
	hr = pType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &v1);
	if (hr != MF_E_ATTRIBUTENOTFOUND)
	{
		checkHR(hr, "get bits per sample");
		ainfo->bitsPerSample = v1;
	}

	// build description
	adesc << ainfo->audioFormatStr;
	if (ainfo->samplesPerSecond >= 0)
		adesc << " "  << ainfo->samplesPerSecond << "hz";
	if (ainfo->channels >= 0)
		adesc << " " << ainfo->channels << "ch";
	if (ainfo->bitsPerSample > 8)
		adesc << " " << ainfo->bitsPerSample << "bits";

	ainfo->description = adesc.str();

	// add to info
	info.audioList().push_back(ainfo);
}

void loadMediaType(MediaInfo &info, int index, CComPtr<IMFMediaType> pType)
{
	GUID majorguid;

	// MAJOR TYPE
	checkHR(pType->GetMajorType(&majorguid), "check major type");

	if (IsEqualGUID(majorguid, MFMediaType_Video))
	{
		loadMediaType_Video(info, index, pType);
	}
	else if (IsEqualGUID(majorguid, MFMediaType_Audio))
	{
		loadMediaType_Audio(info, index, pType);
	}
}

void MediaInfo::load(const std::string &filename)
{
	CComQIPtr<IMFSourceReader> pReader;

	// load source reader from filename
	std::wstring wfilename(filename.begin(), filename.end());
	checkHR(MFCreateSourceReaderFromURL(
		wfilename.c_str(),
		NULL, &pReader), "load filename");

	// enumerate streams
	HRESULT hr = S_OK;
	DWORD dwStreamIndex = 0;
	while (SUCCEEDED(hr))
	{
		// enumerate types for stream
		DWORD dwMediaTypeIndex = 0;

		while (SUCCEEDED(hr))
		{
			CComPtr<IMFMediaType> pType;
			hr = pReader->GetNativeMediaType(dwStreamIndex, dwMediaTypeIndex, &pType);
			if (hr == MF_E_INVALIDSTREAMNUMBER)
				break;
			if (hr == MF_E_NO_MORE_TYPES)
			{
				hr = S_OK;
				break;
			}
			else if (SUCCEEDED(hr))
			{
				loadMediaType(*this, dwStreamIndex, pType);
			}
			else
			{
				checkHR(hr, "GetNativeMediaType");
			}
			++dwMediaTypeIndex;
		}

		if (hr == MF_E_INVALIDSTREAMNUMBER)
		{
			hr = S_OK;
			break;
		}

		dwStreamIndex++;
	}

	checkHR(hr, "read streams");
}

void MediaInfo::load(IMFMediaSource *pSource)
{
}


void checkHR(HRESULT hr, const std::string &operation)
{
	if (FAILED(hr))
	{
		std::stringstream errmsg;
		errmsg << "Error 0x" << std::hex << hr << std::dec << " in operation '" << operation << "'";
		throw Exception(errmsg.str());
	}
}

}