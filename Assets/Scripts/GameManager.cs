using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class GameManager : MonoBehaviour
{
    private Text scoreText;
    private Text gameOverText;
    private bool isGameOver;
    private float startX;
    private Transform dino;

    private void Start()
    {
        var controller = FindFirstObjectByType<DinoController>();
        if (controller != null)
        {
            dino = controller.transform;
            startX = dino.position.x;
        }
    }

    private void Update()
    {
        if (isGameOver)
        {
            if (Input.GetKeyDown(KeyCode.R))
            {
                Time.timeScale = 1f;
                SceneManager.LoadScene(SceneManager.GetActiveScene().name);
            }
            return;
        }

        if (dino == null)
        {
            return;
        }

        var distance = Mathf.Max(0f, dino.position.x - startX);
        if (scoreText != null)
        {
            scoreText.text = $"Score: {Mathf.FloorToInt(distance)}";
        }
    }

    public void InitializeUI(Text score, Text gameOver)
    {
        scoreText = score;
        gameOverText = gameOver;
    }

    public void GameOver()
    {
        if (isGameOver)
        {
            return;
        }

        isGameOver = true;
        if (gameOverText != null)
        {
            gameOverText.text = "Game Over\nPress R to Restart";
        }
        Time.timeScale = 0f;
    }
}
