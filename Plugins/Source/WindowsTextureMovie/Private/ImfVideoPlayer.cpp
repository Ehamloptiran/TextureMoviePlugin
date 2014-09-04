
#include "WindowsTextureMoviePrivatePCH.h"

#pragma comment(lib, "shlwapi")
#pragma comment(lib, "mf")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfplay")
#pragma comment(lib, "mfuuid")

#include "AllowWindowsPlatformTypes.h"

#include <windows.h>
#include <shlwapi.h>
#include <mfapi.h>
#include <mfidl.h>

DEFINE_LOG_CATEGORY( LogImfVideoPlayer );

/** FImfVideoPlayer */

STDMETHODIMP FImfVideoPlayer::QueryInterface( REFIID RefID, void** Object )
{
	static const QITAB QITab[] =
	{
		QITABENT( FImfVideoPlayer, IMFAsyncCallback ),
		{ 0 }
	};

	return QISearch( this, QITab, RefID, Object );
}

STDMETHODIMP_( ULONG ) FImfVideoPlayer::AddRef( )
{
	return FPlatformAtomics::InterlockedIncrement( &RefCount );
}

STDMETHODIMP_( ULONG ) FImfVideoPlayer::Release( )
{
	int64 Ref = FPlatformAtomics::InterlockedDecrement( &RefCount );
	if( Ref == 0 ) { delete this; }

	return Ref;
}

/** Asyncronous callback */
HRESULT FImfVideoPlayer::Invoke( IMFAsyncResult* AsyncResult )
{
	IMFMediaEvent* Event = NULL;
	HRESULT HResult = MediaSession->EndGetEvent( AsyncResult, &Event );

	if( FAILED( HResult ) )
	{
		Event->Release( );
		return S_OK;
	}

	MediaEventType EventType = MEUnknown;
	HResult = Event->GetType( &EventType );
	Event->Release( );

	if( FAILED( HResult ) )
		return S_OK;

	/* Closed */
	if( EventType == MESessionClosed )
	{
		MovieIsFinished.Set( 1 );
		CloseIsPosted.Set( 1 );
	}
	else
	{
		HResult = MediaSession->BeginGetEvent( this, NULL );
		if( FAILED( HResult ) )
			return S_OK;

		if( MovieIsRunning( ) )
		{
			/* End of clip */
			if( EventType == MEEndOfPresentation )
			{
				if( Looping )
					StartPlayback( );

				else
					MovieIsFinished.Set( 1 );
			}

			/* Unknown error, dont continue */
			else if( EventType == MEError )
			{
				HRESULT HReturnCode = S_OK;
				Event->GetStatus( &HReturnCode );

				/* Log error HResult */
				UE_LOG( LogImfVideoPlayer, Log, TEXT( "ImfVideoPlayer error recieved: %i" ), HReturnCode );

				MovieIsFinished.Set( 1 );
				CloseIsPosted.Set( 1 );
			}
		}

		/* DEBUG: Displays all event ID's in log */
		//UE_LOG( LogImfVideoPlayer, Log, TEXT( "ImfVideoPlayer event id: %i" ), EventType );
	}

	return S_OK;
}

/** Opens file from HDD */
FIntPoint FImfVideoPlayer::OpenFile( const FString& FilePath, ECodecMovie::Type CodecType, class FImfSampleGrabberCallback* SampleGrabberCallback )
{
	/* Unimplemented */
	return FIntPoint::ZeroValue;
}

/** Opens file from memory */
FIntPoint FImfVideoPlayer::OpenFile( void* FileSource, int32 Size, ECodecMovie::Type CodecType, FImfSampleGrabberCallback* SampleGrabberCallback )
{
	FIntPoint OutDimensions = FIntPoint::ZeroValue;
	HRESULT HResult = S_OK;

	/* Create Media Session */
	HResult = MFCreateMediaSession( NULL, &MediaSession );
	check( SUCCEEDED( HResult ) );

	/* Begin Get Event */
	HResult = MediaSession->BeginGetEvent( this, NULL );
	check( SUCCEEDED( HResult ) );

	IMFSourceResolver* SourceResolver = NULL;
	IUnknown* Source = NULL;

	/* Create Source Resolver */
	HResult = MFCreateSourceResolver( &SourceResolver );
	check( SUCCEEDED( HResult ) );

	/* Determine codec type */
	FString Ext = ".null";
	switch( CodecType )
	{
	case ECodecMovie::CodecMovie_Wmv:
		Ext = ".wmv";
		break;

	/* Currently not working */
	case ECodecMovie::CodecMovie_Mp4:
		Ext = ".mp4";
		break;

	case ECodecMovie::CodecMovie_Avi:
		Ext = ".avi";
		break;
	}

	/* Create custom ByteStream */
	FImfByteStream* NewByteStream = new FImfByteStream( FileSource, Size );

	/* Create Media Source */
	MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;
	HResult = SourceResolver->CreateObjectFromByteStream( NewByteStream, *Ext, MF_RESOLUTION_MEDIASOURCE, NULL, &ObjectType, &Source );
	SourceResolver->Release( );

	if( SUCCEEDED( HResult ) )
	{
		HResult = Source->QueryInterface( IID_PPV_ARGS( &MediaSource ) );
		Source->Release( );

		/* Set Playback Topology */
		OutDimensions = SetPlaybackTopology( SampleGrabberCallback );
	}
	else
	{
		/* Failed */
		MovieIsFinished.Set( 1 );
	}

	return OutDimensions;
}

/** Sets various playback flags */
void FImfVideoPlayer::SetPlaybackParameters( bool InLooping, bool InOneFrameOnly, bool InResetOnLastFrame )
{
	Looping = InLooping;
	OneFrameOnly = InOneFrameOnly;
	ResetOnLastFrame = InResetOnLastFrame;
}

/** Starts playback */
void FImfVideoPlayer::StartPlayback( )
{
	check( MediaSession != NULL );

	PROPVARIANT VariantStart;
	PropVariantInit( &VariantStart );

	HRESULT HResult = MediaSession->Start( &GUID_NULL, &VariantStart );
	check( SUCCEEDED( HResult ) );

	PropVariantClear( &VariantStart );
}

/** Stops playback */
void FImfVideoPlayer::StopPlayback( )
{
	HRESULT HResult = MediaSession->Stop( );
	check( SUCCEEDED( HResult ) );
}

/** Pauses playback */
void FImfVideoPlayer::PausePlayback( bool Pause )
{
	/* TODO: */
	/* Not every media source can pause. If a media source can pause, the IMFMediaSource::GetCharacteristics method returns the MFMEDIASOURCE_CAN_PAUSE flag. */

	HRESULT HResult = S_OK;
	if( Pause )
	{
		HResult = MediaSession->Pause( );
		check( SUCCEEDED( HResult ) );
	}
	else
	{
		StartPlayback( );
	}
}

/** Shutdown and release resources */
void FImfVideoPlayer::Shutdown( )
{
	MediaSource->Shutdown( );
	MediaSource->Release( );
	MediaSource = NULL;

	MediaSession->Shutdown( );
	MediaSession->Release( );
	MediaSession = NULL;
}

/** Set playback topology */
FIntPoint FImfVideoPlayer::SetPlaybackTopology( FImfSampleGrabberCallback* SampleGrabberCallback )
{
	FIntPoint OutDimensions = FIntPoint( ForceInit );
	HRESULT HResult = S_OK;

	IMFPresentationDescriptor* PresentationDesc = NULL;
	HResult = MediaSource->CreatePresentationDescriptor( &PresentationDesc );
	check( SUCCEEDED( HResult ) );

	IMFTopology* Topology = NULL;
	HResult = MFCreateTopology( &Topology );
	check( SUCCEEDED( HResult ) );

	DWORD StreamCount = 0;
	HResult = PresentationDesc->GetStreamDescriptorCount( &StreamCount );
	check( SUCCEEDED( HResult ) );

	for( uint32 i = 0; i < StreamCount; i++ )
	{
		BOOL bSelected = 0;

		IMFStreamDescriptor* StreamDesc = NULL;
		HResult = PresentationDesc->GetStreamDescriptorByIndex( i, &bSelected, &StreamDesc );
		check( SUCCEEDED( HResult ) );

		if( bSelected )
		{
			FIntPoint VideoDimensions = AddStreamToTopology( Topology, PresentationDesc, StreamDesc, SampleGrabberCallback );
			if( VideoDimensions != FIntPoint( ForceInit ) )
				OutDimensions = VideoDimensions;
		}

		StreamDesc->Release( );
	}

	HResult = MediaSession->SetTopology( 0, Topology );
	check( SUCCEEDED( HResult ) );

	Topology->Release( );
	PresentationDesc->Release( );

	return OutDimensions;
}

/** Add stream to topology */
FIntPoint FImfVideoPlayer::AddStreamToTopology( IMFTopology* Topology, IMFPresentationDescriptor* PresentationDesc, IMFStreamDescriptor* StreamDesc, FImfSampleGrabberCallback* SampleGrabberCallback )
{
	FIntPoint OutDimensions = FIntPoint( ForceInit );
	HRESULT HResult = S_OK;

	IMFActivate* SinkActivate = NULL;
	{
		IMFMediaTypeHandler* Handler = NULL;
		HResult = StreamDesc->GetMediaTypeHandler( &Handler );
		check( SUCCEEDED( HResult ) );

		GUID MajorType;
		HResult = Handler->GetMajorType( &MajorType );
		check( SUCCEEDED( HResult ) );

		/* Audio stream */
		if( MajorType == MFMediaType_Audio )
		{
			/* No audio required */

			Handler->Release( );
			return FIntPoint( ForceInit );
		}

		/* Video stream */
		else if( MajorType == MFMediaType_Video )
		{
			IMFMediaType* OutputType = NULL;
			HResult = Handler->GetCurrentMediaType( &OutputType );
			check( SUCCEEDED( HResult ) );

			IMFMediaType* InputType = NULL;
			HResult = MFCreateMediaType( &InputType );

			UINT32 Width = 0, Height = 0;
			HResult = MFGetAttributeSize( OutputType, MF_MT_FRAME_SIZE, &Width, &Height );
			check( SUCCEEDED( HResult ) );

			HResult = InputType->SetGUID( MF_MT_MAJOR_TYPE, MFMediaType_Video );
			check( SUCCEEDED( HResult ) );
			HResult = InputType->SetGUID( MF_MT_SUBTYPE, MFVideoFormat_RGB32 );
			check( SUCCEEDED( HResult ) );
			HResult = InputType->SetUINT32( MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE );
			check( SUCCEEDED( HResult ) );
			HResult = MFCreateSampleGrabberSinkActivate( InputType, SampleGrabberCallback, &SinkActivate );

			check( SUCCEEDED( HResult ) );
			InputType->Release( );
			OutputType->Release( );

			OutDimensions = FIntPoint( Width, Height );
		}

		Handler->Release( );
	}

	IMFTopologyNode* SourceNode = NULL;
	{
		HResult = MFCreateTopologyNode( MF_TOPOLOGY_SOURCESTREAM_NODE, &SourceNode );
		check( SUCCEEDED( HResult ) );
		HResult = SourceNode->SetUnknown( MF_TOPONODE_SOURCE, MediaSource );
		check( SUCCEEDED( HResult ) );
		HResult = SourceNode->SetUnknown( MF_TOPONODE_PRESENTATION_DESCRIPTOR, PresentationDesc );
		check( SUCCEEDED( HResult ) );
		HResult = SourceNode->SetUnknown( MF_TOPONODE_STREAM_DESCRIPTOR, StreamDesc );
		check( SUCCEEDED( HResult ) );
		HResult = Topology->AddNode( SourceNode );
		check( SUCCEEDED( HResult ) );
	}

	IMFTopologyNode* OutputNode = NULL;
	{
		HResult = MFCreateTopologyNode( MF_TOPOLOGY_OUTPUT_NODE, &OutputNode );
		check( SUCCEEDED( HResult ) );
		HResult = OutputNode->SetObject( SinkActivate );
		check( SUCCEEDED( HResult ) );
		HResult = OutputNode->SetUINT32( MF_TOPONODE_STREAMID, 0 );
		check( SUCCEEDED( HResult ) );
		HResult = OutputNode->SetUINT32( MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, 0 );
		check( SUCCEEDED( HResult ) );
		HResult = Topology->AddNode( OutputNode );
		check( SUCCEEDED( HResult ) );
	}

	HResult = SourceNode->ConnectOutput( 0, OutputNode, 0 );
	check( SUCCEEDED( HResult ) );

	SourceNode->Release( );
	OutputNode->Release( );
	SinkActivate->Release( );

	return OutDimensions;
}

/** FImfSampleGrabberCallback */

STDMETHODIMP FImfSampleGrabberCallback::QueryInterface( REFIID RefID, void** Object )
{
	static const QITAB QITab[] =
	{
		QITABENT( FImfSampleGrabberCallback, IMFSampleGrabberSinkCallback ),
		QITABENT( FImfSampleGrabberCallback, IMFClockStateSink ),
		{ 0 }
	};

	return QISearch( this, QITab, RefID, Object );
}

STDMETHODIMP_( ULONG ) FImfSampleGrabberCallback::AddRef( )
{
	return FPlatformAtomics::InterlockedIncrement( &RefCount );
}

STDMETHODIMP_( ULONG ) FImfSampleGrabberCallback::Release( )
{
	int64 Ref = FPlatformAtomics::InterlockedDecrement( &RefCount );
	if( Ref == 0 ) { delete this; }

	return Ref;
}

/** Process a sample */
STDMETHODIMP FImfSampleGrabberCallback::OnProcessSample( REFGUID MajorMediaType, DWORD SampleFlags, LONGLONG SampleTime, LONGLONG SampleDuration, const BYTE* SampleBuffer, DWORD SampleSize )
{
	if( VideoSampleReady.GetValue( ) == 0 )
	{
		check( TextureData.Num( ) == SampleSize );
		FMemory::Memcpy( &TextureData[ 0 ], SampleBuffer, SampleSize );

		VideoSampleReady.Set( 1 );
	}

	return S_OK;
}

/** Shutdown and release resources */
STDMETHODIMP FImfSampleGrabberCallback::OnShutdown( )
{
	TextureData.Empty( );
	return S_OK;
}

/** Returns TRUE if sample is ready to update */
bool FImfSampleGrabberCallback::GetIsSampleReadyToUpdate( ) const
{
	return VideoSampleReady.GetValue( ) != 0;
}

/** Tells the grabber a new sample is required */
void FImfSampleGrabberCallback::SetNeedNewSample( )
{
	VideoSampleReady.Set( 0 );
}

/** FImfByteStream */

STDMETHODIMP FImfByteStream::QueryInterface( REFIID RefID, void** Object )
{
	static const QITAB QITab[] =
	{
		QITABENT( FImfByteStream, IMFByteStream ),
		QITABENT( FImfByteStream, IMFAsyncCallback ),
		{ 0 }
	};

	return QISearch( this, QITab, RefID, Object );
}

STDMETHODIMP_( ULONG ) FImfByteStream::AddRef( )
{
	return FPlatformAtomics::InterlockedIncrement( &RefCount );
}

STDMETHODIMP_( ULONG ) FImfByteStream::Release( )
{
	int64 Ref = FPlatformAtomics::InterlockedDecrement( &RefCount );
	if( Ref == 0 ) { delete this; }

	return Ref;
}

/** Returns the capabilities of the byte stream */
STDMETHODIMP FImfByteStream::GetCapabilities( DWORD* pdwCapabilities )
{
	FScopeLock ScopeLock( &CriticalSection );

	/* Hardcoded to readable and seekable*/
	*pdwCapabilities = MFBYTESTREAM_IS_READABLE | MFBYTESTREAM_IS_SEEKABLE;

	return S_OK;
}

/** Returns length of stream */
STDMETHODIMP FImfByteStream::GetLength( QWORD* pqwLength )
{
	FScopeLock ScopeLock( &CriticalSection );

	*pqwLength = Length;
	return S_OK;
}

/** Sets length of stream */
STDMETHODIMP FImfByteStream::SetLength( QWORD qwLength )
{
	/* Unimplemented */
	return E_NOTIMPL;
}

/** Returns current position of stream */
STDMETHODIMP FImfByteStream::GetCurrentPosition( QWORD* pqwPosition )
{
	FScopeLock ScopeLock( &CriticalSection );

	*pqwPosition = Position;
	return S_OK;
}

/** Sets current position of stream */
STDMETHODIMP FImfByteStream::SetCurrentPosition( QWORD qwPosition )
{
	FScopeLock ScopeLock( &CriticalSection );

	if( IsCurrentlyReading )
		return S_FALSE;

	Position = (int32) qwPosition;
	return S_OK;
}

/** Begins an asyncronous read */
STDMETHODIMP FImfByteStream::BeginRead( BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState )
{
	HRESULT HResult = S_OK;

	if( !pCallback || !pb )
		return E_INVALIDARG;

	IMFAsyncResult* Result = NULL;
	AsyncReadState* State = new( std::nothrow ) AsyncReadState( pb, cb );
	if( State == NULL )
		return E_OUTOFMEMORY;

	HResult = MFCreateAsyncResult( State, pCallback, punkState, &Result );
	State->Release( );

	if( SUCCEEDED( HResult ) )
	{
		IsCurrentlyReading = true;
		HResult = MFPutWorkItem( MFASYNC_CALLBACK_QUEUE_STANDARD, this, Result );
		Result->Release( );
	}

	return HResult;
}

/** Ends an asyncronous read */
STDMETHODIMP FImfByteStream::EndRead( IMFAsyncResult* pResult, ULONG* pcbRead )
{
	if( !pcbRead )
		return E_INVALIDARG;

	IUnknown* Unknown;
	pResult->GetObject( &Unknown );

	AsyncReadState* State = static_cast<AsyncReadState*>( Unknown );
	*pcbRead = State->GetBytesRead( );
	Unknown->Release( );

	IsCurrentlyReading = false;

	return S_OK;
}

/** Performs actual read */
STDMETHODIMP FImfByteStream::Read( BYTE* pb, ULONG cb, ULONG* pcbRead )
{
	FScopeLock ScopeLock( &CriticalSection );

	ULONG BytesToRead = cb;
	if( ( Position + cb ) > (ULONG) Length )
		BytesToRead = Length - Position;

	if( BytesToRead > 0 )
		FMemory::Memcpy( pb, ( (BYTE*) Source + Position ), BytesToRead );

	if( pcbRead )
		*pcbRead = BytesToRead;

	Position += BytesToRead;

	return S_OK;
}

/** Begins an asyncronous write */
STDMETHODIMP FImfByteStream::BeginWrite( const BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState )
{
	/* Unimplemented */
	return E_NOTIMPL;
}

/** Ends an asyncronous write */
STDMETHODIMP FImfByteStream::EndWrite( IMFAsyncResult* pResult, ULONG* pcbWritten )
{
	/* Unimplemented */
	return E_NOTIMPL;
}

/** Performs actual write */
STDMETHODIMP FImfByteStream::Write( const BYTE* pb, ULONG cb, ULONG* pcbWritten )
{
	/* Unimplemented */
	return E_NOTIMPL;
}

/** Performs a seek */
STDMETHODIMP FImfByteStream::Seek( MFBYTESTREAM_SEEK_ORIGIN SeekOrigin, LONGLONG qwSeekOffset, DWORD dwSeekFlags, QWORD* pqwCurrentPosition )
{
	FScopeLock ScopeLock( &CriticalSection );

	/* Don't seek if an asyncronous read is in progress */
	if( IsCurrentlyReading )
		return S_FALSE;

	if( SeekOrigin == msoCurrent )
		Position += qwSeekOffset;
	else
		Position = qwSeekOffset;

	if( pqwCurrentPosition )
		*pqwCurrentPosition = Position;

	return S_OK;
}

/** Flush the stream */
STDMETHODIMP FImfByteStream::Flush( )
{
	/* Unimplemented */
	return E_NOTIMPL;
}

/** Closes the stream */
STDMETHODIMP FImfByteStream::Close( )
{
	FScopeLock ScopeLock( &CriticalSection );

	/* Nothing to do */

	return S_OK;
}

/** Returns TRUE if at end of stream */
STDMETHODIMP FImfByteStream::IsEndOfStream( BOOL* pfEndOfStream )
{
	if( !pfEndOfStream )
		return E_INVALIDARG;

	FScopeLock ScopeLock( &CriticalSection );

	*pfEndOfStream = ( Position >= Length ) ? TRUE : FALSE;

	return S_OK;
}

/** Gets parameters */
STDMETHODIMP FImfByteStream::GetParameters( DWORD* pdwFlags, DWORD* pdwQueue )
{
	/* Unimplemented */
	return E_NOTIMPL;
}

/** Asyncronous callback */
STDMETHODIMP FImfByteStream::Invoke( IMFAsyncResult* pResult )
{
	HRESULT HResult = S_OK;

	IUnknown* State = NULL;
	IUnknown* Unknown = NULL;
	IMFAsyncResult* CallerResult = NULL;

	AsyncReadState* Op = NULL;

	HResult = pResult->GetState( &State );
	check( SUCCEEDED( HResult ) );

	HResult = State->QueryInterface( IID_PPV_ARGS( &CallerResult ) );
	check( SUCCEEDED( HResult ) );

	HResult = CallerResult->GetObject( &Unknown );
	check( SUCCEEDED( HResult ) );

	Op = static_cast<AsyncReadState*>( Unknown );

	/* Do actual read */
	ULONG cbRead;
	Read( Op->GetBuffer( ), Op->GetCount( ) - Op->GetBytesRead( ), &cbRead );
	Op->SetBytesRead( cbRead + Op->GetBytesRead( ) );

	if( CallerResult )
	{
		CallerResult->SetStatus( HResult );
		MFInvokeCallback( CallerResult );
	}

	if( State != NULL ) { State->Release( ); }
	if( Unknown != NULL ) { Unknown->Release( ); }
	if( CallerResult != NULL ){ CallerResult->Release( ); }

	return S_OK;
}

/** FImfByteStream::ASyncReadState */

STDMETHODIMP FImfByteStream::AsyncReadState::QueryInterface( REFIID RefID, void** Object )
{
	static const QITAB QITab[] =
	{
		QITABENT( AsyncReadState, IUnknown ),
		{ 0 }
	};

	return QISearch( this, QITab, RefID, Object );
}

STDMETHODIMP_( ULONG ) FImfByteStream::AsyncReadState::AddRef( )
{
	return FPlatformAtomics::InterlockedIncrement( &RefCount );
}

STDMETHODIMP_( ULONG ) FImfByteStream::AsyncReadState::Release( )
{
	int64 Ref = FPlatformAtomics::InterlockedDecrement( &RefCount );
	if( Ref == 0 ) { delete this; }

	return Ref;
}

#include "HideWindowsPlatformTypes.h"
