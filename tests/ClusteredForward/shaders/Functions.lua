local dimensions = {
    NumLights = 2048;
    TileWidth = 64;
    TileHeight = 64;
};

function dimensions.TileCountX()
    return (GetWindowX() - 1) / dimensionFunctions.TileWidth + 1;
end

function dimensions.TileCountY()
    return (GetWindowY() - 1) / dimensionFunctions.TileHeight + 1;
end

function dimensions.TileCountZ()
    return 256;
end

function dimensions.GetTileSizes()
    return dimensions.TileCountX(), dimensions.TileCountY(), dimensions.TileCountZ();
end

function dimensions.GetTotalTileCount()
    x, y, z = dimensions.GetTileSizes();
    return x * y * z;
end

return dimensions;
