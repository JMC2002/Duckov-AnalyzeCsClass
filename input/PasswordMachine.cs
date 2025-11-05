using System.Collections.Generic;
using Duckov.Quests;
using SodaCraft.Localizations;
using UnityEngine;

public class PasswordMachine : MonoBehaviour
{
	public List<Condition> conditions;

	[Tooltip("从下到上")]
	public List<int> rightCode;

	private List<int> nums = new List<int>();

	[LocalizationKey("Default")]
	[SerializeField]
	private string wrongTimeKey = "Passworld_WrongTime";

	[LocalizationKey("Default")]
	[SerializeField]
	private string wrongPassWorldKey = "Passworld_WrongNumber";

	[LocalizationKey("Default")]
	[SerializeField]
	private string rightKey = "Passworld_Right";

	[SerializeField]
	private DialogueBubbleProxy dialogueBubbleProxy;

	[SerializeField]
	private GameObject activeObject;

	[SerializeField]
	private BoxCollider interactBoxCollider;

	private float popSpeed = 30f;

	public int maxNum => rightCode.Count;

	private void Start()
	{
	}

	private void Update()
	{
	}

	private bool CheckConditions()
	{
		if (conditions.Count == 0)
		{
			return true;
		}
		for (int i = 0; i < conditions.Count; i++)
		{
			if (!(conditions[i] == null) && !conditions[i].Evaluate())
			{
				return false;
			}
		}
		return true;
	}

	public void InputNum(int num)
	{
		if (nums.Count < maxNum)
		{
			nums.Add(num);
		}
		dialogueBubbleProxy.Pop(CurrentNums(), popSpeed);
	}

	public void DeleteNum()
	{
		if (nums.Count > 0)
		{
			nums.RemoveAt(nums.Count - 1);
		}
		dialogueBubbleProxy.Pop(CurrentNums(), popSpeed);
	}

	public void Confirm()
	{
		if (rightCode.Count != nums.Count)
		{
			nums.Clear();
			dialogueBubbleProxy.Pop(wrongPassWorldKey.ToPlainText(), popSpeed);
			return;
		}
		for (int i = 0; i < rightCode.Count; i++)
		{
			if (rightCode[i] != nums[i])
			{
				nums.Clear();
				dialogueBubbleProxy.Pop(wrongPassWorldKey.ToPlainText(), popSpeed);
				return;
			}
		}
		if (!CheckConditions())
		{
			nums.Clear();
			dialogueBubbleProxy.Pop(wrongTimeKey.ToPlainText(), popSpeed);
			return;
		}
		nums.Clear();
		dialogueBubbleProxy.Pop(rightKey.ToPlainText(), popSpeed);
		activeObject.SetActive(value: true);
		interactBoxCollider.enabled = false;
	}

	private string CurrentNums()
	{
		string text = "";
		for (int i = 0; i < nums.Count; i++)
		{
			text += nums[i];
		}
		for (int j = 0; j < maxNum - nums.Count; j++)
		{
			text += "_";
		}
		return text;
	}
}
