package main

func main() {
	n := readInt()
	i, j := 0, 0 // номер строки и столбца

	if n > 10 {
		println("Invalid input value (number must be less than 10)")
	} else {
		matrix := [10][10]int{}

		for a := 1; a < n*n+1; a++ {
			matrix[i][j] = a
			if i <= j+1 && (i+j < n-1) {
				j += 1
			} else if i < j && (i+j >= n-1) {
				i += 1
			} else if i >= j && (i+j > n-1) {
				j -= 1
			} else if i > j && (i+j <= n-1) {
				i -= 1
			}
		}

		for i := 0; i < n; i++ {
			for j := 0; j < n; j++ {
				print(matrix[i][j])
				print(" ")
			}
			println("")
		}
	}
}
