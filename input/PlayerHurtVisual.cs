using UnityEngine;
using UnityEngine.Rendering;

public class PlayerHurtVisual : MonoBehaviour
{
	[SerializeField]
	private Volume volume;

	[SerializeField]
	private float speed = 4f;

	private Health mainCharacterHealth;

	private bool inited;

	public static bool hurtVisualOn = true;

	private float value;

	private void Update()
	{
		if (!inited)
		{
			TryInit();
			return;
		}
		value = Mathf.MoveTowards(value, 0f, Time.deltaTime * speed);
		if (volume.weight != value)
		{
			volume.weight = value;
		}
	}

	private void TryInit()
	{
		if (!LevelManager.LevelInited)
		{
			return;
		}
		CharacterMainControl main = CharacterMainControl.Main;
		if ((bool)main)
		{
			mainCharacterHealth = main.Health;
			if ((bool)mainCharacterHealth)
			{
				mainCharacterHealth.OnHurtEvent.AddListener(OnHurt);
				inited = true;
			}
		}
	}

	private void OnDestroy()
	{
		if ((bool)mainCharacterHealth)
		{
			mainCharacterHealth.OnHurtEvent.RemoveListener(OnHurt);
		}
	}

	private void OnHurt(DamageInfo dmgInfo)
	{
		if (!(dmgInfo.damageValue < 1.5f))
		{
			if (!mainCharacterHealth || !hurtVisualOn)
			{
				value = 0f;
			}
			else if (mainCharacterHealth.CurrentHealth / mainCharacterHealth.MaxHealth > 0.5f)
			{
				value = 0.5f;
			}
			else
			{
				value = 1f;
			}
			if (volume.weight != value)
			{
				volume.weight = value;
			}
		}
	}
}
