using System;
using SodaCraft.Localizations;
using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;

namespace Duckov.Quests.UI;

public class QuestSortButton : MonoBehaviour, IPointerClickHandler, IEventSystemHandler
{
	[Serializable]
	private struct Entry
	{
		[LocalizationKey("Default")]
		public string displayNameKey;

		public Quest.SortingMode mode;
	}

	[SerializeField]
	private TextMeshProUGUI text;

	[SerializeField]
	private MonoBehaviour targetBehaviour;

	[SerializeField]
	private Entry[] entries;

	private int index;

	private IQuestSortable target;

	public Quest.SortingMode SortingMode
	{
		get
		{
			if (entries.Length == 0)
			{
				Debug.LogError("Error: Entries not configured for sorting mode button of quest ui.");
				return Quest.SortingMode.Default;
			}
			if (index < 0)
			{
				index = 0;
			}
			else if (index >= entries.Length)
			{
				index = 0;
			}
			return entries[index].mode;
		}
	}

	private void Start()
	{
		Refresh();
		if (!(targetBehaviour == null) && targetBehaviour is IQuestSortable questSortable)
		{
			target = questSortable;
		}
	}

	public void OnPointerClick(PointerEventData eventData)
	{
		eventData.Use();
		index++;
		if (index >= entries.Length)
		{
			index = 0;
		}
		Refresh();
		Apply();
	}

	private void Refresh()
	{
		if (entries.Length != 0 && index >= 0 && index < entries.Length)
		{
			Entry entry = entries[index];
			text.text = entry.displayNameKey.ToPlainText();
		}
	}

	private void Apply()
	{
		target.SortingMode = SortingMode;
	}
}
