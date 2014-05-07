
#include "WindowsTextureMoviePrivatePCH.h"

#if WITH_EDITOR

/** Constructor */
UTextureMovieFactory::UTextureMovieFactory( const class FPostConstructInitializeProperties& PCIP )
	: Super( PCIP )
{
	SupportedClass = UTextureMovie::StaticClass( );

	Formats.Add( TEXT( "wmv;WMV Video" ) );
	//Formats.Add( TEXT( "mp4;MP4 Video" ) );
	Formats.Add( TEXT( "avi;AVI Video" ) );

	bCreateNew = false;
	bEditorImport = true;
}

/** Post init properties */
void UTextureMovieFactory::PostInitProperties( )
{
	Super::PostInitProperties( );
}

/** Does support class*/
bool UTextureMovieFactory::DoesSupportClass( UClass* Class )
{
	return Class == UTextureMovie::StaticClass( );
}

/** Create binary */
UObject* UTextureMovieFactory::FactoryCreateBinary(
	UClass* Class,
	UObject* InParent,
	FName InName,
	EObjectFlags Flags,
	UObject* Context,
	const TCHAR* Type,
	const uint8*& Buffer,
	const uint8* BufferEnd,
	FFeedbackContext* Warn
	)
{
	/* Create new TextureMovie*/
	UTextureMovie* MovieTexture = ConstructObject<UTextureMovie>( Class, InParent, InName, Flags );
	if( MovieTexture )
	{
		FString TypeStr = Type;

		/* WMV Video file */
		if( TypeStr.Equals( TEXT( "wmv" ) ) )
			MovieTexture->DecoderClass = UCodecMovieWmv::StaticClass( );

		/* AVI Video file */
		else if( TypeStr.Equals( TEXT( "avi" ) ) )
			MovieTexture->DecoderClass = UCodecMovieAvi::StaticClass( );

		/* Send data to codec */
		MovieTexture->Data.Lock( LOCK_READ_WRITE );
		void* LockedData = MovieTexture->Data.Realloc( BufferEnd - Buffer );
		FMemory::Memcpy( LockedData, Buffer, BufferEnd - Buffer );
		MovieTexture->Data.Unlock( );
	}

	return MovieTexture;
}

#endif