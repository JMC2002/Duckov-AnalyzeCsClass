using System.Collections.Generic;
using Duckov.Options;
using UnityEngine;

public class HideIfSickFriendly : MonoBehaviour
{
	public List<GameObject> hideList = new List<GameObject>();

	private bool sickFriendly;

	private void Start()
	{
		Sync();
		OptionsManager.OnOptionsChanged += OnOptionsChanged;
	}

	private void OnDestroy()
	{
		OptionsManager.OnOptionsChanged -= OnOptionsChanged;
	}

	private void OnOptionsChanged(string option)
	{
		Sync();
	}

	private void Sync()
	{
		bool disableCameraOffset = DisableCameraOffset.disableCameraOffset;
		if (sickFriendly != disableCameraOffset)
		{
			sickFriendly = disableCameraOffset;
		}
		foreach (GameObject hide in hideList)
		{
			if ((bool)hide)
			{
				hide.SetActive(!sickFriendly);
			}
		}
	}
}
