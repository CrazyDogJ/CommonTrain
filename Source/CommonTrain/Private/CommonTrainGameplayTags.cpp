#include "CommonTrainGameplayTags.h"

namespace CommonTrainTags
{
	UE_DEFINE_GAMEPLAY_TAG(Forward,		FName{TEXTVIEW("CommonTrain.State.Forward")})
	UE_DEFINE_GAMEPLAY_TAG(Stopping,	FName{TEXTVIEW("CommonTrain.State.Stopping")})
	UE_DEFINE_GAMEPLAY_TAG(StopIntro,	FName{TEXTVIEW("CommonTrain.State.StopIntro")})
	UE_DEFINE_GAMEPLAY_TAG(StopOutro,	FName{TEXTVIEW("CommonTrain.State.StopOutro")})
	UE_DEFINE_GAMEPLAY_TAG(Backward,	FName{TEXTVIEW("CommonTrain.State.Backward")})
}
