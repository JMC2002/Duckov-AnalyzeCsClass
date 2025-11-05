namespace Duckov.Quests.UI;

public interface IQuestSortable
{
	Quest.SortingMode SortingMode { get; set; }

	bool SortRevert { get; set; }
}
