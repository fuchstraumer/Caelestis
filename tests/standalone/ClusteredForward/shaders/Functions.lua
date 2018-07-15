local dimensions = {
    NumLights = 2048;
    TileWidth = 64;
    TileHeight = 64;
};

function dimensions.TileCountX()
    return (GetWindowX() - 1) / dimensions.TileWidth + 1;
end

function dimensions.TileCountY()
    return (GetWindowY() - 1) / dimensions.TileHeight + 1;
end

function dimensions.TileCountZ()
    return 256;
end

function dimensions.TileSizes()
    return dimensions.TileCountX(), dimensions.TileCountY(), dimensions.TileCountZ();
end

function dimensions.TotalTileCount()
    x, y, z = dimensions.TileSizes();
    return x * y * z;
end

return dimensions;
