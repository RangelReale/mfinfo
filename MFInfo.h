#ifndef __H_MFINFO_H__
#define __H_MFINFO_H__

#include <Windows.h>
#include <mfidl.h>

#include <memory>
#include <vector>
#include <string>
#include <stdexcept>

namespace mfinfo {

class Exception : public std::runtime_error
{
public:
	Exception(const std::string &message) : std::runtime_error(message) {}
};

class VideoInfo_H264
{
public:
	typedef std::shared_ptr<VideoInfo_H264> Ptr;

	VideoInfo_H264() : description(), profile(-1), profileStr("Unknown"), level(-1), levelStr("Unknown"), cabac(false) {}

	std::string description;

	int profile; // eAVEncH264VProfile_XXX
	std::string profileStr;

	int level; // eAVEncH264VLevelXXXX
	std::string levelStr;

	bool cabac;
};

class VideoInfo
{
public:
	typedef std::shared_ptr<VideoInfo> Ptr;

	VideoInfo() : index(-1), description(), videoFormat(), videoFormatStr("Unknown"), width(-1), height(-1), 
		frameRate(-1), bitRate(-1), bitRateStr("Unknown"),
		info_h264() {}

	int index;
	std::string description;

	GUID videoFormat; // MFVideoFormat_XXX
	std::string videoFormatStr;

	int width;
	int height;
	double frameRate;
	
	int bitRate;
	std::string bitRateStr;

	VideoInfo_H264::Ptr info_h264;
};

class AudioInfo
{
public:
	typedef std::shared_ptr<AudioInfo> Ptr;

	AudioInfo() : index(-1), description(), audioFormat(), audioFormatStr("Unknown"), channels(-1), samplesPerSecond(-1), bitsPerSample(-1) {}

	int index;
	std::string description;

	GUID audioFormat; // MFAudioFormat_XXX
	std::string audioFormatStr;

	int channels;
	int samplesPerSecond;
	int bitsPerSample;
};

class MediaInfo
{
public:
	typedef std::vector<VideoInfo::Ptr> videolist_t;
	typedef std::vector<AudioInfo::Ptr> audiolist_t;

	MediaInfo();

	void clear();

	void load(const std::string &filename);
	void load(IMFMediaSource *pSource);

	videolist_t &videoList() { return _videolist; }
	audiolist_t &audioList() { return _audiolist; }
private:
	videolist_t _videolist;
	audiolist_t _audiolist;
};

void checkHR(HRESULT hr, const std::string &operation);

}

#endif // __H_MFINFO_H__