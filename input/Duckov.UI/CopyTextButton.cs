using UnityEngine;
using UnityEngine.EventSystems;

namespace Duckov.UI;

public class CopyTextButton : MonoBehaviour, IPointerClickHandler, IEventSystemHandler
{
	[SerializeField]
	private string text;

	public void OnPointerClick(PointerEventData eventData)
	{
		GUIUtility.systemCopyBuffer = text;
	}
}
