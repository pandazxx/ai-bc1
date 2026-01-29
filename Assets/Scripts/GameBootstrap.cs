using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class GameBootstrap : MonoBehaviour
{
    [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.BeforeSceneLoad)]
    private static void EnsureBootstrap()
    {
        var existing = FindFirstObjectByType<GameBootstrap>();
        if (existing != null)
        {
            return;
        }

        var bootstrap = new GameObject("GameBootstrap");
        bootstrap.AddComponent<GameBootstrap>();
        DontDestroyOnLoad(bootstrap);
    }

    private void Awake()
    {
        if (FindFirstObjectByType<GameManager>() != null)
        {
            return;
        }

        BuildScene();
    }

    private void BuildScene()
    {
        var gameManager = new GameObject("GameManager");
        gameManager.AddComponent<GameManager>();

        var dino = new GameObject("Dino");
        dino.transform.position = new Vector3(0f, 1.25f, 0f);
        var dinoSprite = dino.AddComponent<SpriteRenderer>();
        dinoSprite.sprite = PixelArtUtility.CreateSprite(new Color(0.25f, 0.8f, 0.25f), 24);
        var dinoCollider = dino.AddComponent<BoxCollider2D>();
        dinoCollider.size = new Vector2(1f, 1.25f);
        var dinoBody = dino.AddComponent<Rigidbody2D>();
        dinoBody.freezeRotation = true;
        dino.AddComponent<DinoController>();

        var ground = new GameObject("Ground");
        ground.transform.position = new Vector3(0f, 0f, 0f);
        var groundSprite = ground.AddComponent<SpriteRenderer>();
        groundSprite.sprite = PixelArtUtility.CreateSprite(new Color(0.18f, 0.18f, 0.18f), 16);
        groundSprite.drawMode = SpriteDrawMode.Tiled;
        groundSprite.size = new Vector2(50f, 1f);
        var groundCollider = ground.AddComponent<BoxCollider2D>();
        groundCollider.size = new Vector2(50f, 1f);

        var spawner = new GameObject("ObstacleSpawner");
        spawner.AddComponent<ObstacleSpawner>();

        var cameraObject = new GameObject("Main Camera");
        var camera = cameraObject.AddComponent<Camera>();
        camera.orthographic = true;
        camera.orthographicSize = 5.5f;
        camera.backgroundColor = new Color(0.9f, 0.95f, 1f);
        var cameraFollow = cameraObject.AddComponent<CameraFollow>();
        cameraFollow.Target = dino.transform;

        SetupUI();
    }

    private void SetupUI()
    {
        var canvas = new GameObject("HUD");
        var canvasComponent = canvas.AddComponent<Canvas>();
        canvasComponent.renderMode = RenderMode.ScreenSpaceOverlay;
        var scaler = canvas.AddComponent<CanvasScaler>();
        scaler.uiScaleMode = CanvasScaler.ScaleMode.ScaleWithScreenSize;
        scaler.referenceResolution = new Vector2(800, 450);
        canvas.AddComponent<GraphicRaycaster>();

        var scoreText = CreateUIText("ScoreText", canvas.transform, new Vector2(16, -16), TextAnchor.UpperLeft);
        scoreText.text = "Score: 0";

        var gameOverText = CreateUIText("GameOverText", canvas.transform, Vector2.zero, TextAnchor.MiddleCenter);
        gameOverText.text = "";
        gameOverText.fontSize = 32;

        var manager = FindFirstObjectByType<GameManager>();
        if (manager != null)
        {
            manager.InitializeUI(scoreText, gameOverText);
        }
    }

    private static Text CreateUIText(string name, Transform parent, Vector2 anchoredPosition, TextAnchor alignment)
    {
        var textObject = new GameObject(name);
        textObject.transform.SetParent(parent, false);
        var text = textObject.AddComponent<Text>();
        text.font = Resources.GetBuiltinResource<Font>("Arial.ttf");
        text.color = new Color(0.12f, 0.12f, 0.12f);
        text.alignment = alignment;

        var rect = text.GetComponent<RectTransform>();
        rect.anchorMin = new Vector2(0f, 1f);
        rect.anchorMax = new Vector2(0f, 1f);
        rect.pivot = new Vector2(0f, 1f);
        rect.anchoredPosition = anchoredPosition;
        rect.sizeDelta = new Vector2(400f, 100f);

        if (alignment == TextAnchor.MiddleCenter)
        {
            rect.anchorMin = new Vector2(0.5f, 0.5f);
            rect.anchorMax = new Vector2(0.5f, 0.5f);
            rect.pivot = new Vector2(0.5f, 0.5f);
            rect.anchoredPosition = anchoredPosition;
            rect.sizeDelta = new Vector2(600f, 200f);
        }

        return text;
    }
}
