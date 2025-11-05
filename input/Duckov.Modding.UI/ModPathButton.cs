using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;

namespace Duckov.Modding.UI;

public class ModPathButton : MonoBehaviour, IPointerClickHandler, IEventSystemHandler
{
	[SerializeField]
	private TextMeshProUGUI pathText;

	private void OnEnable()
	{
		pathText.text = ModManager.DefaultModFolderPath;
	}

	public void OnPointerClick(PointerEventData eventData)
	{
		GUIUtility.systemCopyBuffer = ModManager.DefaultModFolderPath;
	}
}
