using System.Collections.Generic;
using System.Linq;
using Cysharp.Threading.Tasks;
using Duckov.Economy;
using Duckov.Utilities;
using ItemStatsSystem;
using ItemStatsSystem.Items;
using UnityEngine;

[CreateAssetMenu]
public class DecomposeDatabase : ScriptableObject
{
	[SerializeField]
	private DecomposeFormula[] entries;

	private Dictionary<int, DecomposeFormula> _dic;

	public static DecomposeDatabase Instance => GameplayDataSettings.DecomposeDatabase;

	private Dictionary<int, DecomposeFormula> Dic
	{
		get
		{
			if (_dic == null)
			{
				RebuildDictionary();
			}
			return _dic;
		}
	}

	public void RebuildDictionary()
	{
		_dic = new Dictionary<int, DecomposeFormula>();
		DecomposeFormula[] array = entries;
		for (int i = 0; i < array.Length; i++)
		{
			DecomposeFormula value = array[i];
			_dic[value.item] = value;
		}
	}

	public DecomposeFormula GetFormula(int itemTypeID)
	{
		if (!Dic.TryGetValue(itemTypeID, out var value))
		{
			return default(DecomposeFormula);
		}
		return value;
	}

	public static async UniTask<bool> Decompose(Item item, int count)
	{
		if (Instance == null)
		{
			return false;
		}
		DecomposeFormula formula = Instance.GetFormula(item.TypeID);
		if (!formula.valid)
		{
			return false;
		}
		Item splitedItem = item;
		if (item.Stackable)
		{
			int stackCount = item.StackCount;
			if (stackCount <= count)
			{
				count = stackCount;
			}
			else
			{
				splitedItem = await item.Split(count);
			}
		}
		bool dontMerge = PlayerStorage.Instance != null;
		if (splitedItem.Slots != null)
		{
			foreach (Slot slot in splitedItem.Slots)
			{
				if (slot != null)
				{
					Item content = slot.Content;
					if (!(content == null))
					{
						content.Detach();
						ItemUtilities.SendToPlayer(content, dontMerge);
					}
				}
			}
		}
		if (splitedItem.Inventory != null)
		{
			foreach (Item item2 in splitedItem.Inventory.Where((Item e) => e != null).ToList())
			{
				if (!(item2 == null))
				{
					item2.Detach();
					ItemUtilities.SendToPlayer(item2, dontMerge);
				}
			}
		}
		Cost result = formula.result;
		await result.Return(directToBuffer: false, toPlayerInventory: true, (!splitedItem.Stackable) ? 1 : splitedItem.StackCount);
		splitedItem.Detach();
		splitedItem.DestroyTree();
		return true;
	}

	public static bool CanDecompose(int itemTypeID)
	{
		if (Instance == null)
		{
			return false;
		}
		return Instance.GetFormula(itemTypeID).valid;
	}

	public static bool CanDecompose(Item item)
	{
		if (item == null)
		{
			return false;
		}
		return CanDecompose(item.TypeID);
	}

	public static DecomposeFormula GetDecomposeFormula(int itemTypeID)
	{
		if (Instance == null)
		{
			return default(DecomposeFormula);
		}
		return Instance.GetFormula(itemTypeID);
	}

	public void SetData(List<DecomposeFormula> formulas)
	{
		entries = formulas.ToArray();
	}
}
