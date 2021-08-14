VERTEX_1 = [(-0.5, -0.5, 0.0),
            (0.5, -0.5, 0.0),
            (0.5, 0.5, 0.0),
            (-0.5, 0.5, 0.0)]

VERTEX_2 = [(-0.5, -0.5, -0.5),
            (0.5, -0.5, -0.5),
            (0.5, 0.5, -0.5),
            (-0.5, 0.5, -0.5)]

indices1 = [0, 1, 2, 2, 3, 0]

indices2 = [4, 5, 6, 6, 7, 4]


index = 0


def calculate_indices(vertex):
    global index

    verts = []
    indices = []

    for vert in vertex:
        if vert not in verts:
            indices.append(index)
            verts.append(vert)
            index += 1
        if len(verts) % 3 == 0:
            indices.append(index-1)

    indices.append(indices[0])

    return indices


if __name__ == "__main__":
    indis = []

    indis += calculate_indices(VERTEX_1)
    print(indis)
    print(indis[0:6] == indices1)

    indis += calculate_indices(VERTEX_2)
    print(indis[6:])
    print(indis[6:] == indices2)

    print(indis)
