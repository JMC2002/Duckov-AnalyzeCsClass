using System.Collections.Generic;
using Cysharp.Threading.Tasks;
using Duckov.Quests;
using UnityEngine;

public class SetActiveByCondition : MonoBehaviour
{
	public GameObject targetObject;

	public bool inverse;

	public List<Condition> conditions;

	public bool autoCheck = true;

	public bool update;

	public bool requireLevelInited = true;

	private float checkTimeSpace = 1f;

	private void Update()
	{
		if ((LevelManager.LevelInited || !requireLevelInited) && autoCheck)
		{
			Set();
			if (update)
			{
				CheckAndLoop().Forget();
			}
			base.enabled = false;
		}
	}

	public void Set()
	{
		if ((bool)targetObject)
		{
			bool flag = conditions.Satisfied();
			if (inverse)
			{
				flag = !flag;
			}
			targetObject.SetActive(flag);
		}
	}

	private async UniTaskVoid CheckAndLoop()
	{
		await UniTask.WaitForSeconds(checkTimeSpace);
		if (!(this == null))
		{
			Set();
			CheckAndLoop().Forget();
		}
	}
}
