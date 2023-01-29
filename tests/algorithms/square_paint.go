package main

func paint(matrix [][]int, curX, curY, maxX, maxY int) {
	if curX > maxX {
		return

	} else if curX < 0 {
		return

	} else if curY > maxY {
		return

	} else if curY < 0 {
		return
	}

	if matrix[curX][curY] > 0 {
		return
	}

	matrix[curX][curY] = 2

	paint(matrix, curX-1, curY, maxX, maxY)
	paint(matrix, curX, curY-1, maxX, maxY)
	paint(matrix, curX+1, curY, maxX, maxY)
	paint(matrix, curX, curY+1, maxX, maxY)
}

func main() {
	matrix := [][]int{
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 1, 1, 1, 1, 1, 1, 0, 1},
		{1, 0, 1, 0, 0, 0, 0, 1, 0, 1},
		{1, 0, 1, 0, 1, 1, 1, 1, 0, 1},
		{1, 0, 1, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	}

	paint(matrix, 2, 1, 9, 9)

	for _, array := range matrix {
		println(array)
	}
}
