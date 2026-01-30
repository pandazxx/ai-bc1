using UnityEngine;

public static class PixelArtUtility
{
    public static Sprite CreateSprite(Color color, int size, int pixelsPerUnit = 16)
    {
        var texture = new Texture2D(size, size, TextureFormat.RGBA32, false);
        var pixels = new Color[size * size];
        for (int i = 0; i < pixels.Length; i++)
        {
            pixels[i] = color;
        }
        texture.SetPixels(pixels);
        texture.filterMode = FilterMode.Point;
        texture.wrapMode = TextureWrapMode.Clamp;
        texture.Apply();

        var rect = new Rect(0, 0, size, size);
        var pivot = new Vector2(0.5f, 0.5f);
        return Sprite.Create(texture, rect, pivot, pixelsPerUnit);
    }
}
