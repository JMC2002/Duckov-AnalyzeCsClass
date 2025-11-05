using Duckov.Buffs;
using ItemStatsSystem;
using UnityEngine;

public class ItemSetting_MeleeWeapon : ItemSettingBase
{
	public bool dealExplosionDamage;

	public ElementTypes element;

	[Range(0f, 1f)]
	public float buffChance;

	public Buff buff;

	public override void Start()
	{
		base.Start();
	}

	public override void SetMarkerParam(Item selfItem)
	{
		selfItem.SetBool("IsMeleeWeapon", value: true);
	}
}
