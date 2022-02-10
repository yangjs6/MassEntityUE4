
namespace UE::AI
{
	/**
	 * This method will extract the yaw radian from the specified vector (The vector does not need to be normalized)
	 * if it is not possible to compute yaw, the function will return an invalid value	 *
	 */
	extern MASSCOMMON_API TOptional<float> GetYawFromVector(const FVector& Vector);

	/**
	 * This method will extract the yaw radian from the specified rotator
	 * if it is not possible to compute yaw, the function will not return an invalid value
	 */
	extern MASSCOMMON_API TOptional<float> GetYawFromRotator(const FRotator& Rotator);

	/**
	 * This method will extract the yaw radian from the specified quaternion
	 * if it is not possible to compute yaw, the function will not return an invalid value
	 */
	extern MASSCOMMON_API TOptional<float> GetYawFromQuaternion(const FQuat& Quaternion);

	/**
	 * Fetches all the components of ActorClass's CDO, including the ones added via the BP editor (which AActor.GetComponents fails to do)
	 * @param ActorClass class of AActor for which we will retrieve all components
	 * @param OutComponents this is where the found components will end up. Note that the preexisting contents of OutComponents will get overridden.
	 * @param InComponentClass if supplied will be used to filter the results
	 */
	extern MASSCOMMON_API void GetActorClassDefaultComponents(const TSubclassOf<AActor>& ActorClass, TArray<UActorComponent*>& OutComponents, const TSubclassOf<UActorComponent>& InComponentClass );
} // UE::AI
