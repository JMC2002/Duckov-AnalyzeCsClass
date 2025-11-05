using System;
using System.Collections.Generic;
using System.Linq;
using Duckov.UI;
using Duckov.UI.Animations;
using Duckov.Utilities;
using UnityEngine;

namespace Duckov.Quests.UI;

public class QuestView : View, ISingleSelectionMenu<QuestEntry>, IQuestSortable
{
	[Flags]
	public enum ShowContent
	{
		Active = 1,
		History = 2
	}

	[SerializeField]
	private QuestEntry questEntry;

	[SerializeField]
	private Transform questEntryParent;

	[SerializeField]
	private GameObject entryListPlaceHolder;

	[SerializeField]
	private QuestViewDetails details;

	[SerializeField]
	private FadeGroup fadeGroup;

	private QuestManager target;

	[SerializeField]
	private ShowContent showingContentType;

	private Quest.SortingMode _sortingMode;

	private bool _sortRevert;

	private PrefabPool<QuestEntry> _questEntryPool;

	private List<QuestEntry> activeEntries = new List<QuestEntry>();

	private QuestEntry selectedQuestEntry;

	public static QuestView Instance => View.GetViewInstance<QuestView>();

	public ShowContent ShowingContentType => showingContentType;

	public Quest.SortingMode SortingMode
	{
		get
		{
			return _sortingMode;
		}
		set
		{
			_sortingMode = value;
			RefreshEntryList();
		}
	}

	public bool SortRevert
	{
		get
		{
			return _sortRevert;
		}
		set
		{
			_sortRevert = value;
			RefreshEntryList();
		}
	}

	public IList<Quest> ShowingContent
	{
		get
		{
			if (target == null)
			{
				return null;
			}
			return showingContentType switch
			{
				ShowContent.Active => target.ActiveQuests, 
				ShowContent.History => target.HistoryQuests, 
				_ => null, 
			};
		}
	}

	private PrefabPool<QuestEntry> QuestEntryPool
	{
		get
		{
			if (_questEntryPool == null)
			{
				_questEntryPool = new PrefabPool<QuestEntry>(questEntry, questEntryParent, delegate(QuestEntry e)
				{
					activeEntries.Add(e);
					e.SetMenu(this);
				}, delegate(QuestEntry e)
				{
					activeEntries.Remove(e);
				});
			}
			return _questEntryPool;
		}
	}

	private QuestEntry SelectedQuestEntry => selectedQuestEntry;

	public Quest SelectedQuest => selectedQuestEntry?.Target;

	internal event Action<QuestView, ShowContent> onShowingContentChanged;

	internal event Action<QuestView, QuestEntry> onSelectedEntryChanged;

	public void Setup()
	{
		Setup(QuestManager.Instance);
	}

	private void Setup(QuestManager target)
	{
		this.target = target;
		Quest oldSelection = SelectedQuest;
		RefreshEntryList();
		QuestEntry questEntry = activeEntries.Find((QuestEntry e) => e.Target == oldSelection);
		if (questEntry != null)
		{
			SetSelection(questEntry);
		}
		else
		{
			SetSelection(null);
		}
		RefreshDetails();
	}

	public static void Show()
	{
		Instance?.Open();
	}

	protected override void OnOpen()
	{
		base.OnOpen();
		Setup();
		fadeGroup.Show();
	}

	protected override void OnClose()
	{
		base.OnClose();
		fadeGroup.Hide();
	}

	protected override void Awake()
	{
		base.Awake();
	}

	private void OnEnable()
	{
		RegisterStaticEvents();
		Setup(QuestManager.Instance);
	}

	private void OnDisable()
	{
		if (details != null)
		{
			details.Setup(null);
		}
		UnregisterStaticEvents();
	}

	private void RegisterStaticEvents()
	{
		QuestManager.onQuestListsChanged += Setup;
	}

	private void UnregisterStaticEvents()
	{
		QuestManager.onQuestListsChanged -= Setup;
	}

	private void RefreshEntryList()
	{
		QuestEntryPool.ReleaseAll();
		bool flag = target != null && ShowingContent != null && ShowingContent.Count > 0;
		entryListPlaceHolder.SetActive(!flag);
		if (!flag)
		{
			return;
		}
		List<Quest> list = ShowingContent.ToList();
		if (SortingMode != Quest.SortingMode.Default)
		{
			list.Sort((Quest a, Quest b) => Quest.Compare(a, b, SortingMode, SortRevert));
		}
		foreach (Quest item in list)
		{
			QuestEntry obj = QuestEntryPool.Get(questEntryParent);
			obj.Setup(item);
			obj.transform.SetAsLastSibling();
		}
	}

	private void RefreshDetails()
	{
		details.Setup(SelectedQuest);
	}

	public void SetShowingContent(ShowContent flags)
	{
		showingContentType = flags;
		RefreshEntryList();
		List<QuestEntry> list = activeEntries;
		if (list != null && list.Count > 0)
		{
			SetSelection(activeEntries[0]);
		}
		else
		{
			SetSelection(null);
		}
		RefreshDetails();
		foreach (QuestEntry activeEntry in activeEntries)
		{
			activeEntry.NotifyRefresh();
		}
		this.onShowingContentChanged?.Invoke(this, flags);
	}

	public void ShowActiveQuests()
	{
		SetShowingContent(ShowContent.Active);
	}

	public void ShowHistoryQuests()
	{
		SetShowingContent(ShowContent.History);
	}

	public QuestEntry GetSelection()
	{
		return selectedQuestEntry;
	}

	public bool SetSelection(QuestEntry selection)
	{
		selectedQuestEntry = selection;
		this.onSelectedEntryChanged?.Invoke(this, selectedQuestEntry);
		foreach (QuestEntry activeEntry in activeEntries)
		{
			activeEntry.NotifyRefresh();
		}
		RefreshDetails();
		return true;
	}
}
