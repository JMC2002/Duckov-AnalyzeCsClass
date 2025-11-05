using ItemStatsSystem;
using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

public class CraftView_ListEntry : MonoBehaviour, IPointerClickHandler, IEventSystemHandler
{
	[SerializeField]
	private Color normalColor;

	[SerializeField]
	private Color normalInsufficientColor;

	[SerializeField]
	private Image icon;

	[SerializeField]
	private Image background;

	[SerializeField]
	private TextMeshProUGUI nameText;

	[SerializeField]
	private GameObject selectedIndicator;

	public CraftView Master { get; private set; }

	public CraftingFormula Formula { get; private set; }

	private void OnEnable()
	{
		ItemUtilities.OnPlayerItemOperation += Refresh;
	}

	private void OnDisable()
	{
		ItemUtilities.OnPlayerItemOperation -= Refresh;
	}

	public void Setup(CraftView master, CraftingFormula formula)
	{
		Master = master;
		Formula = formula;
		ItemMetaData metaData = ItemAssetsCollection.GetMetaData(Formula.result.id);
		icon.sprite = metaData.icon;
		nameText.text = $"{metaData.DisplayName} x{formula.result.amount}";
		Refresh();
	}

	public void OnPointerClick(PointerEventData eventData)
	{
		Master?.SetSelection(this);
	}

	internal void NotifyUnselected()
	{
		Refresh();
	}

	internal void NotifySelected()
	{
		Refresh();
	}

	private void Refresh()
	{
		if (!(Master == null))
		{
			bool active = Master.GetSelection() == this;
			Color color = normalColor;
			if (selectedIndicator != null)
			{
				selectedIndicator.SetActive(active);
			}
			color = ((!Formula.cost.Enough) ? normalInsufficientColor : normalColor);
			background.color = color;
		}
	}
}
