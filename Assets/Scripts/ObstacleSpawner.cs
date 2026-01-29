using System.Collections.Generic;
using UnityEngine;

public class ObstacleSpawner : MonoBehaviour
{
    public float spawnDistanceAhead = 18f;
    public float minSpacing = 4f;
    public float maxSpacing = 7f;

    private readonly Queue<GameObject> pool = new Queue<GameObject>();
    private Transform dino;
    private float nextSpawnX;

    private void Start()
    {
        var controller = FindFirstObjectByType<DinoController>();
        if (controller != null)
        {
            dino = controller.transform;
            nextSpawnX = dino.position.x + spawnDistanceAhead;
        }
    }

    private void Update()
    {
        if (dino == null)
        {
            return;
        }

        if (dino.position.x + spawnDistanceAhead >= nextSpawnX)
        {
            SpawnObstacle(nextSpawnX);
            nextSpawnX += Random.Range(minSpacing, maxSpacing);
        }

        CleanupObstacles();
    }

    private void SpawnObstacle(float xPosition)
    {
        var obstacle = pool.Count > 0 ? pool.Dequeue() : CreateObstacle();
        obstacle.SetActive(true);

        var width = Random.Range(0.6f, 1.2f);
        var height = Random.Range(1f, 2f);
        obstacle.transform.position = new Vector3(xPosition, height * 0.5f, 0f);

        var renderer = obstacle.GetComponent<SpriteRenderer>();
        renderer.sprite = PixelArtUtility.CreateSprite(new Color(0.1f, 0.6f, 0.1f), 16);
        renderer.drawMode = SpriteDrawMode.Sliced;
        renderer.size = new Vector2(width, height);

        var collider = obstacle.GetComponent<BoxCollider2D>();
        collider.size = new Vector2(width, height);

        var obstacleComponent = obstacle.GetComponent<Obstacle>();
        obstacleComponent.width = width;
    }

    private GameObject CreateObstacle()
    {
        var obstacle = new GameObject("Obstacle");
        obstacle.AddComponent<Obstacle>();
        var renderer = obstacle.AddComponent<SpriteRenderer>();
        renderer.sprite = PixelArtUtility.CreateSprite(new Color(0.1f, 0.6f, 0.1f), 16);
        renderer.drawMode = SpriteDrawMode.Sliced;
        obstacle.AddComponent<BoxCollider2D>();
        return obstacle;
    }

    private void CleanupObstacles()
    {
        var obstacles = FindObjectsByType<Obstacle>(FindObjectsSortMode.None);
        foreach (var obstacle in obstacles)
        {
            if (obstacle.transform.position.x < dino.position.x - 12f)
            {
                obstacle.gameObject.SetActive(false);
                pool.Enqueue(obstacle.gameObject);
            }
        }
    }
}
