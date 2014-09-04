
#pragma once

#include "AllowWindowsPlatformTypes.h"
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>

DECLARE_LOG_CATEGORY_EXTERN( LogImfVideoPlayer, Log, All );

/* Media foundation video player */

class FImfVideoPlayer : public IMFAsyncCallback
{
public:

	/** Default constructor */
	FImfVideoPlayer( )
		: RefCount( 1 )
		, MediaSession( NULL )
		, MediaSource( NULL )
		, MovieIsFinished( 0 )
		, CloseIsPosted( 0 )
	{
	}

	virtual ~FImfVideoPlayer( )
	{
	}

	/* Begin IUnknown interface */
	STDMETHODIMP QueryInterface( REFIID RefID, void** Object );
	STDMETHODIMP_( ULONG ) AddRef( );
	STDMETHODIMP_( ULONG ) Release( );
	/* End IUnknown interface */

	/* Begin IMFAsyncCallback interface */
	STDMETHODIMP GetParameters( unsigned long*, unsigned long* ) { return E_NOTIMPL; }
	STDMETHODIMP Invoke( IMFAsyncResult* AsyncResult );
	/* End IMFAsyncCallback interface */

	/** Open file from HDD */
	FIntPoint OpenFile( const FString& FilePath, ECodecMovie::Type CodecType, class FImfSampleGrabberCallback* SampleGrabberCallback );

	/** Open file from memory */
	FIntPoint OpenFile( void* Source, int32 Size, ECodecMovie::Type CodecType, class FImfSampleGrabberCallback* SampleGrabberCallback );

	/** Sets various playback flags */
	void SetPlaybackParameters( bool InLooping, bool InOneFrameOnly, bool InResetOnLastFrame );

	/** Starts playback */
	void StartPlayback( );

	/** Stops playback */
	void StopPlayback( );

	/** Pauses and resumes playback */
	void PausePlayback( bool Pause );

	/** Shutdown and release resources */
	void Shutdown( );

	/** Returns true if movie is running */
	bool MovieIsRunning( ) const { return MovieIsFinished.GetValue( ) == 0; }

private:

	/** Sets playback topology */
	FIntPoint SetPlaybackTopology( class FImfSampleGrabberCallback* SampleGrabberCallback );

	/** Adds stream to topology */
	FIntPoint AddStreamToTopology( IMFTopology* Topology, IMFPresentationDescriptor* PresentationDesc, IMFStreamDescriptor* StreamDesc, class FImfSampleGrabberCallback* SampleGrabberCallback );

private:

	int64 RefCount;

	/** Media session reference */
	IMFMediaSession* MediaSession;

	/** Media source reference */
	IMFMediaSource* MediaSource;

	/** Thread safe variables */
	FThreadSafeCounter MovieIsFinished;
	FThreadSafeCounter CloseIsPosted;

	/** Various playback flags */
	bool Looping;
	bool OneFrameOnly;
	bool ResetOnLastFrame;
};

/** Media foundation Sample Grabber Callback */

class FImfSampleGrabberCallback : public IMFSampleGrabberSinkCallback
{
public:

	/** Default constructor */
	FImfSampleGrabberCallback( TArray<uint8>& InTextureData )
		: RefCount( 1 )
		, TextureData( InTextureData )
	{
	}

	/* Begin IUnknown interface */
	STDMETHODIMP QueryInterface( REFIID RefID, void** Object );
	STDMETHODIMP_( ULONG ) AddRef( );
	STDMETHODIMP_( ULONG ) Release( );
	/* End IUnknown interface */

	/* Begin IMFClockStateSink interface */
	STDMETHODIMP OnClockStart( MFTIME SystemTime, LONGLONG llClockStartOffset ) { return S_OK; }
	STDMETHODIMP OnClockStop( MFTIME SystemTime ) { return S_OK; }
	STDMETHODIMP OnClockPause( MFTIME SystemTime ) { return S_OK; }
	STDMETHODIMP OnClockRestart( MFTIME SystemTime ) { return S_OK; }
	STDMETHODIMP OnClockSetRate( MFTIME SystemTime, float flRate ) { return S_OK; }
	/* End IMFClockStateSink interface */

	/* Begin IMFSampleGrabberSinkCallback interface */
	STDMETHODIMP OnSetPresentationClock( IMFPresentationClock* Clock ) { return S_OK; }
	STDMETHODIMP OnProcessSample(
		REFGUID guidMajorMediaType,
		DWORD dwSampleFlags,
		LONGLONG llSampleTime,
		LONGLONG llSampleDuration,
		const BYTE *pSampleBuffer,
		DWORD dwSampleSize
		);
	STDMETHODIMP OnShutdown( );
	/* End IMFSampleGrabberSinkCallback interface */

	/** Returns true if sample is ready to update */
	bool GetIsSampleReadyToUpdate( ) const;

	/** Tells the grabber that a new sample is required */
	void SetNeedNewSample( );

private:

	int64 RefCount;

	/** TRUE if video sample is ready */
	FThreadSafeCounter VideoSampleReady;

	/** Texture data array */
	TArray<uint8>& TextureData;
};

/** Media Foundation byte stream */
/** For reading files from memory */

class FImfByteStream : public IMFByteStream, public IMFAsyncCallback
{
public:

	/** Default constructor */
	FImfByteStream( void* InSource, int32 InLength )
		: RefCount( 1 )
		, Source( InSource )
		, Length( InLength )
		, Position( 0 )
		, IsCurrentlyReading( false )
	{
	}

	/* Begin IUnknown interface */
	STDMETHODIMP QueryInterface( REFIID RefID, void** Object );
	STDMETHODIMP_( ULONG ) AddRef( );
	STDMETHODIMP_( ULONG ) Release( );
	/* End IUnknown interface */

	/* Begin IMFByteStream interface */
	STDMETHODIMP GetCapabilities( DWORD* pdwCapabilities );
	STDMETHODIMP GetLength( QWORD* pqwLength );
	STDMETHODIMP SetLength( QWORD qwLength );
	STDMETHODIMP GetCurrentPosition( QWORD* pqwPosition );
	STDMETHODIMP SetCurrentPosition( QWORD qwPosition );
	STDMETHODIMP BeginRead( BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState );
	STDMETHODIMP EndRead( IMFAsyncResult* pResult, ULONG* pcbRead );
	STDMETHODIMP Read( BYTE* pb, ULONG cb, ULONG* pcbRead );
	STDMETHODIMP BeginWrite( const BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState );
	STDMETHODIMP EndWrite( IMFAsyncResult* pResult, ULONG* pcbWritten );
	STDMETHODIMP Write( const BYTE* pb, ULONG cb, ULONG* pcbWritten );
	STDMETHODIMP Seek( MFBYTESTREAM_SEEK_ORIGIN SeekOrigin, LONGLONG qwSeekOffset, DWORD dwSeekFlags, QWORD* pqwCurrentPosition );
	STDMETHODIMP Flush( );
	STDMETHODIMP Close( );
	STDMETHODIMP IsEndOfStream( BOOL* pfEndOfStream );
	/* End IMFByteStream interface */

	/* Begin IMFAsyncCallback interface */
	STDMETHODIMP GetParameters( DWORD* pdwFlags, DWORD* pdwQueue );
	STDMETHODIMP Invoke( IMFAsyncResult* pResult );
	/* End IMFASyncCallback interface */

private:

	/** Asynchronous values for callback */
	class AsyncReadState : public IUnknown
	{
	public:

		AsyncReadState( BYTE* InBuf, ULONG InCount )
			: RefCount( 1 )
			, Buffer( InBuf )
			, Count( InCount )
			, BytesRead( 0 )
		{
		}

		/* Begin IUnknown interface */
		STDMETHODIMP QueryInterface( REFIID RefID, void** Object );
		STDMETHODIMP_( ULONG ) AddRef( );
		STDMETHODIMP_( ULONG ) Release( );
		/* End IUnknown interface */

		BYTE* GetBuffer( ) const { return Buffer; }
		ULONG GetCount( ) const { return Count; }
		ULONG GetBytesRead( ) const { return BytesRead; }

		void SetBytesRead( ULONG cbRead ) { BytesRead = cbRead; }

	private:

		int64 RefCount;
		BYTE* Buffer;
		ULONG Count;
		ULONG BytesRead;
	};

protected:

	/** TRUE if an asyncronous read is currently being performed */
	bool IsCurrentlyReading;

private:
	
	int64 RefCount;
	void* Source;
	int32 Length;
	int32 Position;

	mutable FCriticalSection CriticalSection;
};

#include "HideWindowsPlatformTypes.h"