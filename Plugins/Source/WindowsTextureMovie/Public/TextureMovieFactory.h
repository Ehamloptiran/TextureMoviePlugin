
#pragma once

#if WITH_EDITOR

#include "TextureMovieFactory.generated.h"

/** TextureMovie Factory */
UCLASS( CustomConstructor, CollapseCategories, HideCategories = Object )
class UTextureMovieFactory : public UFactory
{
	GENERATED_UCLASS_BODY( )

public:

	/** Default constructor */
	UTextureMovieFactory( const class FPostConstructInitializeProperties& PCIP );

	/* Begin UObject interface */
	virtual void PostInitProperties( );
	/* End UObject interface */

	/* Begin UFactory interface */
	virtual bool DoesSupportClass( UClass* Class ) OVERRIDE;
	virtual UObject* FactoryCreateBinary( 
		UClass* Class, 
		UObject* InParent, 
		FName InName, 
		EObjectFlags Flags, 
		UObject* Context, 
		const TCHAR* Type, 
		const uint8*& Buffer, 
		const uint8* BufferEnd, 
		FFeedbackContext* Warn 
		) OVERRIDE;
	/* End UFactory interface */
};

#endif