using System;
using System.Collections.Generic;
using System.IO;
using Duckov;
using Saves;
using SodaCraft.Localizations;
using SodaCraft.StringUtilities;
using UnityEngine;

public class BaseBGMSelector : MonoBehaviour
{
	[Serializable]
	public struct Entry
	{
		public string musicName;

		public string author;

		public string switchName;

		public string filePath;
	}

	[SerializeField]
	private string switchGroupName = "BGM";

	[SerializeField]
	private DialogueBubbleProxy proxy;

	public List<Entry> entries;

	private int index;

	private const string savekey = "BaseBGMSelector";

	private bool waitForStinger = true;

	[LocalizationKey("Default")]
	private string BGMInfoFormatKey
	{
		get
		{
			return "BGMInfoFormat";
		}
		set
		{
		}
	}

	private string BGMInfoFormat => BGMInfoFormatKey.ToPlainText();

	private void Awake()
	{
		SavesSystem.OnCollectSaveData += Save;
		Load();
		ScanCustomBGM();
	}

	private void OnDestroy()
	{
		SavesSystem.OnCollectSaveData -= Save;
	}

	private void ScanCustomBGM()
	{
		string path = Path.Combine(Application.streamingAssetsPath, "Music");
		string searchPattern = "*.mp3";
		foreach (string item2 in Directory.EnumerateFiles(path, searchPattern))
		{
			ParseMusicFileName(Path.GetFileNameWithoutExtension(item2), out var musicName, out var authorName);
			Entry item = new Entry
			{
				musicName = musicName,
				author = authorName,
				filePath = item2
			};
			entries.Add(item);
		}
	}

	private static void ParseMusicFileName(string fileName, out string musicName, out string authorName)
	{
		musicName = fileName;
		authorName = "Unknown";
		if (fileName.Contains('-'))
		{
			int num = fileName.LastIndexOf('-');
			authorName = fileName.Substring(0, num).Trim();
			if (num + 1 < fileName.Length)
			{
				musicName = fileName.Substring(num + 1).Trim();
			}
		}
	}

	private void Update()
	{
		if (waitForStinger && LevelManager.AfterInit && !AudioManager.IsStingerPlaying)
		{
			waitForStinger = false;
			Set(index);
		}
	}

	private void Load(bool play = false)
	{
		index = SavesSystem.Load<int>("BaseBGMSelector");
	}

	private void Save()
	{
		SavesSystem.Save("BaseBGMSelector", index);
	}

	public void Set(int index, bool showInfo = false, bool play = true)
	{
		waitForStinger = false;
		if (index < 0 || index >= entries.Count)
		{
			int num = index;
			index = Mathf.Clamp(index, 0, entries.Count - 1);
			Debug.LogError($"[BGM Selector] Index {num} Out Of Range,clampped to {index}");
		}
		Entry entry = entries[index];
		AudioManager.StopBGM();
		if (play)
		{
			if (string.IsNullOrWhiteSpace(entry.filePath))
			{
				AudioManager.PlayBGM(entry.switchName);
			}
			else
			{
				AudioManager.PlayCustomBGM(entry.filePath);
			}
		}
		if (showInfo)
		{
			string text = BGMInfoFormat.Format(new
			{
				name = entry.musicName,
				author = entry.author,
				index = index + 1
			});
			proxy.Pop(text, 200f);
		}
	}

	public void Set(string switchName)
	{
		int num = GetIndex(switchName);
		if (num >= 0)
		{
			Set(num);
		}
	}

	public int GetIndex(string switchName)
	{
		for (int i = 0; i < entries.Count; i++)
		{
			if (entries[i].switchName == switchName)
			{
				return i;
			}
		}
		return -1;
	}

	public void SetNext()
	{
		index++;
		if (index >= entries.Count)
		{
			index = 0;
		}
		Set(index, showInfo: true);
	}

	public void SetPrevious()
	{
		index--;
		if (index < 0)
		{
			index = entries.Count - 1;
		}
		Set(index, showInfo: true);
	}
}
