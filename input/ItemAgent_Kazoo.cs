using Duckov;
using Duckov.Utilities;
using FMOD.Studio;
using Unity.Mathematics;
using UnityEngine;

public class ItemAgent_Kazoo : DuckovItemAgent
{
	private string audioEvent = "SFX/Special/Kazoo";

	private EventInstance? currentEvent;

	private bool currentMakingSound;

	private bool targetMakingSound;

	private float makeAiSoundCoolTimer = 0.15f;

	private float makeAiSoundCoolTime = 0.15f;

	public float maxScale = 15f;

	public float maxSoundRange = 18f;

	private Camera camera;

	private bool holderInited;

	private GameObject uiInstance;

	private float zOffset = 6f;

	[SerializeField]
	private ParticleSystem particle;

	private void Update()
	{
		if (!base.Holder)
		{
			return;
		}
		if (!camera)
		{
			if ((bool)GameCamera.Instance)
			{
				camera = GameCamera.Instance.renderCamera;
			}
			if (!camera)
			{
				return;
			}
		}
		if (!holderInited)
		{
			base.Holder.OnTriggerInputUpdateEvent += OnTriggerUpdate;
			uiInstance = Object.Instantiate(GameplayDataSettings.Prefabs.KazooUi, base.Holder.transform.position, quaternion.identity);
			uiInstance.transform.localScale = Vector3.one * 2f * maxScale;
			SyncUi(base.Holder.transform);
			holderInited = true;
		}
		if (targetMakingSound != currentMakingSound)
		{
			makeAiSoundCoolTimer = makeAiSoundCoolTime;
			currentMakingSound = targetMakingSound;
			if (currentMakingSound)
			{
				if (currentEvent.HasValue)
				{
					currentEvent.Value.stop(STOP_MODE.ALLOWFADEOUT);
				}
				currentEvent = AudioManager.Post(audioEvent, base.gameObject);
				if ((bool)particle)
				{
					particle.Emit(1);
				}
			}
			else if (currentEvent.HasValue)
			{
				currentEvent.Value.stop(STOP_MODE.ALLOWFADEOUT);
			}
		}
		if (currentMakingSound)
		{
			Vector3 right = camera.transform.right;
			right.y = 0f;
			right.Normalize();
			Vector3 position = base.Holder.transform.position;
			Vector3 rhs = base.Holder.GetCurrentAimPoint() - position;
			rhs.y = 0f;
			float value = Vector3.Dot(right, rhs) * 24f / maxScale;
			AudioManager.SetRTPC("Kazoo/Pitch", value, base.gameObject);
			AudioManager.SetRTPC("Kazoo/Intensity", 1f, base.gameObject);
			makeAiSoundCoolTimer -= Time.deltaTime;
			if (makeAiSoundCoolTimer <= 0f)
			{
				makeAiSoundCoolTimer = makeAiSoundCoolTime;
				AIMainBrain.MakeSound(new AISound
				{
					fromCharacter = base.Holder,
					fromObject = base.gameObject,
					pos = base.transform.position,
					fromTeam = base.Holder.Team,
					soundType = SoundTypes.unknowNoise,
					radius = maxSoundRange
				});
			}
		}
	}

	private void LateUpdate()
	{
		if ((bool)base.Holder)
		{
			SyncUi(base.Holder.transform);
		}
	}

	public void OnTriggerUpdate(bool trigger, bool triggerThisFrame, bool releaseThisFrame)
	{
		targetMakingSound = trigger;
	}

	protected override void OnInitialize()
	{
		base.OnInitialize();
	}

	private void SyncUi(Transform parent)
	{
		if ((bool)uiInstance && (bool)parent)
		{
			Vector3 forward = camera.transform.forward;
			forward.y = 0f;
			forward.Normalize();
			uiInstance.transform.position = parent.position - forward * zOffset;
			uiInstance.transform.rotation = quaternion.LookRotation(forward, Vector3.up);
		}
	}

	protected override void OnDestroy()
	{
		base.OnDestroy();
		if ((bool)uiInstance)
		{
			Object.Destroy(uiInstance.gameObject);
		}
		if ((bool)base.Holder)
		{
			base.Holder.OnTriggerInputUpdateEvent -= OnTriggerUpdate;
		}
	}

	private void OnDisable()
	{
		if (currentEvent.HasValue)
		{
			currentEvent.Value.stop(STOP_MODE.ALLOWFADEOUT);
		}
	}
}
