using UnityEngine;

public class DinoController : MonoBehaviour
{
    public float forwardSpeed = 6f;
    public float jumpForce = 9f;
    public float groundCheckDistance = 0.1f;

    private Rigidbody2D body;
    private BoxCollider2D collider2d;
    private bool jumpQueued;

    private void Awake()
    {
        body = GetComponent<Rigidbody2D>();
        collider2d = GetComponent<BoxCollider2D>();
    }

    private void Update()
    {
        if (Input.GetKeyDown(KeyCode.Space) || Input.GetKeyDown(KeyCode.UpArrow))
        {
            jumpQueued = true;
        }
    }

    private void FixedUpdate()
    {
        body.linearVelocity = new Vector2(forwardSpeed, body.linearVelocity.y);

        if (jumpQueued && IsGrounded())
        {
            body.linearVelocity = new Vector2(body.linearVelocity.x, jumpForce);
        }

        jumpQueued = false;
    }

    private bool IsGrounded()
    {
        var origin = (Vector2)transform.position + Vector2.down * (collider2d.size.y * 0.5f);
        var hit = Physics2D.Raycast(origin, Vector2.down, groundCheckDistance);
        return hit.collider != null;
    }

    private void OnCollisionEnter2D(Collision2D collision)
    {
        if (collision.collider.GetComponent<Obstacle>() != null)
        {
            var manager = FindFirstObjectByType<GameManager>();
            if (manager != null)
            {
                manager.GameOver();
            }
        }
    }
}
