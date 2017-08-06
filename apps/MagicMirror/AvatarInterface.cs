using System;
using System.Runtime.InteropServices;
using Vixen;

namespace MagicMirror
{
	[
		Guid("ac9fe57a-5185-4e25-bf3a-43c226d584f2"),
		ComVisible(true)
	]
	/*!
	 * @interface Avatar 
	 * Interface for an animatable 3D avatar that you can dress in different outfits.
	 *
	 * @see AvatarScene AvatarEvents
	 */
	public interface Avatar
	{
	#region Methods
		/*!
		 * @param url	URL of 3D scene file to load.
		 * Load a 3D content file (Havok or Vixen format)
		 * and handle a SceneLoadEvent when load is complete.
		 */
		void	LoadScene(String url);

		/*!
		 * @param name	name to assign to loaded avatar.
		 * @param url	URL of 3D avatar content to load.
		 * Load a 3D avatar content file (Havok or Vixen format)
		 * and handle a LoadAvatarEvent when load is complete.
		 */
		void	LoadAvatar(String name, String url);
	
		/*!
		 * Display the named avatar. This function will only display the avatar
		 * if it has already been loaded. If the name is null, the current avatar is shown.
		 * 
		 * @see LoadAvatar HideAvatar
		 */
		void	ShowAvatar(String name);

		/*!
		 * Hide the named avatar if it is currently displayed.
		 * If the name is null, the current avatar is hidden.
		 * 
		 * @see LoadAvatar ShowAvatar
		 */
		void	HideAvatar();

		/*!
		 * @param url		URL to 3D animation file to load.
		 * @param target	name of avatar/garment to apply animation to
		 *					if null, the current avatar is assumed
		 * Load a 3D animation content file (BVH, Havok or Vixen format)
		 * and apply it to the named avatar or garment. The name of the
		 * animation is the base name of the file without the extension.
		 * 
		 * @see PlayAnimation PauseAnimation
		 */
		void LoadAnimation(String url, String target);

		/*!
		 * Plays the named animation on the current avatar.
		 * The animation will not be played if it has not already been loaded.
		 * @param name		name of the animation to play.
		 * @param blend		time to blend to this animation (0 = no blending).
		 * 
		 * @see LoadAnimation PauseAnimation RandomAnimation
		 */
		void	PlayAnimation(String name, float blend);

		/*!
		 * Pauses the currently playing animation.
		 * 
		 * @see LoadAnimation PlayAnimation
		 */
		void	PauseAnimation(String name);

		/*!
		 * @param name	name to assign to loaded garment.
		 * @param url	URL to 3D content file for garment.
		 * Load a 3D clothing content file (Havok format)
		 * and handle a LoadGarmentEvent when load is complete.
		 */
		void LoadGarment(String name, String url);

		/*!
		 * Display the named garment on the avatar. This function will only display the clothing
		 * if it has already been loaded.
		 * 
		 * @see LoadGarment HideGarment
		 */
		void	ShowGarment(String name);

		/*!
		 * Hides the named garment if it is currently draped on the avatar.
		 * 
		 * @see ShowGarment LoadGarment
		 */
		void	HideGarment(String name);

		/*!
		 * Load a closet description from a URL in JSON format.
		 * The garments in the closet are defined but their 3D content is not loaded.
		 * 
		 * @see LoadGarment
		 */
		void	LoadCloset(String url);

		/*!
		 * Load application configuration information from a URL in JSON format.
		 * The garments in the closet are defined but their 3D content is not loaded.
		 * 
		 * @see LoadGarment
		 */
		dynamic	LoadConfig(String url);

		/*
		 * Select and play a random animation from the named category.
		 * @param category	animation category
		 * @param blend		time to blend to this animation (0 = no blending)
		 * 
		 * The animation categories are defined in the JSON config
		 * file in the "animation" section.
		 */
		string RandomAnimation(string category, float blend);

	#endregion
	}

	[
		Guid("fd61f677-f4cc-4cd4-a0dc-47a2c11675d0"),
		ComVisible(true),
		InterfaceType(ComInterfaceType.InterfaceIsIDispatch)
	]
	/*!
	 * @class AvatarEvents
	 * Encapsulates the events emitted by the Avatar interface.
	 * Events occur when content is loaded and when animations are stopped.
	 */
	public interface AvatarEvents
	{
		#region Events
		[DispId(1)]
		/*!
		 * @param name	base name of the scene loaded (without ".scene" suffix)
		 * @param url	full path to the scene content file loaded
		 * Event generated when a 3D content file has been loaded.
		 */
		void OnLoadScene(System.String name, String url);

		[DispId(2)]
		/*!
		 * @param name	name of the Engine object which stopped
		 * *			(the name of the animation)
		 * Event generated when a 3D animation has stopped.
		 */
		void OnStop(System.String name);

		[DispId(3)]
		/*!
		 * @param name	base name of the scene loaded (without ".scene" suffix)
		 * Event generated when the current 3D scene has started to display.
		 */
		void OnSetScene(System.String name);

		[DispId(4)]
		/*!
		 * Event generated when the 3D window has been initialized and is ready to display.
		 */
		void OnInitScene();

		[DispId(5)]
		/*!
		 * @param name	name of avatar loaded (same name as passed to LoadAvatar)
		 * Event generated when the a 3D avatar content file has finished loading.
		 */
		void OnLoadAvatar(System.String name);

		[DispId(6)]
		/*!
		 * @param name	name of garment loaded (same name as passed to LoadGarment)
		 * Event generated when a 3D clothing content file has finished loading.
		 */
		void OnLoadGarment(System.String name);

		[DispId(7)]
		/*!
		 * @param name name of garment which was picked
		 * Event generated when a garment is chosen by the user.
		 */
		void OnSelectGarment(System.String name);

		[DispId(8)]
		/*!
		 * @param closet	ArrayList with clothing items in closet
		 * Event is generated when a closet configuration file has finished loading.
		 * The garments in the closet may not be loaded yet.
		 */
		void OnLoadCloset(System.String url, System.Collections.ArrayList items);

		[DispId(9)]
		/*!
		 * @param message	error message
		 * @param level		error severity level (0 = warning)
		 * Event is generated when an error occurs in the avatar system.
		 * Usually this is a file which failed to load.
		 */
		void OnError(System.String message, int level);
		#endregion
	}

	[
		Guid("0B7335BF-2261-4434-8AB2-54F9169FFC45"),
		ComVisible(true),
		InterfaceType(ComInterfaceType.InterfaceIsIDispatch)
	]
	/*!
	 * These are the events generated by the body tracking sensor
	 * if it is enabled.
	 *
	 * @see KinectAvatarScene
	 */
	public interface BodyTrackEvents
	{
		#region Events
		[DispId(10)]
		/*!
		 * Event generated when the body tracking sensor has initialized.
		 */
		void OnSensorInit();

		[DispId(11)]
		/*!
		 * @param name		name of Vixen engine for sensor
		 * @param userid	sensor ID of new user
		 * Event generated when the sensor detects a new user walking into the scene.
		 * @see OnStartTrack
		 */
		void OnNewUser(System.String name, int userid);

		[DispId(12)]
		/*!
		 * @param name		name of Vixen engine for sensor
		 * @param userid	sensor ID of user
		 * Event generated when the sensor starts tracking a user.
		 * @see OnStopTrack OnPauseTrack
		 */
		void OnStartTrack(System.String name, int userid);

		[DispId(13)]
		/*!
		 * @param name		name of Vixen engine for sensor
		 * @param userid	sensor ID of user
		 * Event generated when the sensor stops tracking a user
		 * (user has left sensor area).
		 * 
		 * @see OnStartTrack OnPauseTrack
		 */
		void OnStopTrack(System.String name, int userid);

		[DispId(14)]
		/*!
		 * @param name		name of Vixen engine for sensor
		 * @param userid	sensor ID of user
		 * Event generated when the sensor pauses tracking the user
		 * 
		 * @see OnStartTrack OnStopTrack
		 */
		void OnPauseTrack(System.String name, int userid);

		[DispId(15)]
		/*!
		 * @param name		name of Vixen engine for sensor
		 * @param userid	sensor ID of user
		 * @param pos		position of users chest
		 * Event generated when the user turns away from the sensor.
		 * 
		 * @see OnStartTrack OnStopTrack
		 */
		void OnUserTurn(System.String name, int userid, Vec3 pos);

		#endregion
	}

}
