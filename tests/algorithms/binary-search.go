package main

func printArray(array []int) {
	for i := 0; i < len(array); i++ {
		print(array[i])
		print(" ")
	}
	println(" ")
}

func main() {
	searchValue := 0

	arr := []int{1, 5, 100, 0, -100, 15, 4, 102, 30, 1000}

	printArray(arr)

	// Sort the numbers
	var tmp = 0
	for i := 0; i < len(arr); i++ {
		for j := 0; j < len(arr)-1; j++ {
			if arr[j] > arr[j+1] {
				tmp = arr[j]
				arr[j] = arr[j+1]
				arr[j+1] = tmp
			}
		}
	}

	printArray(arr)

	left := 0
	right := len(arr) - 1

	if right < left {
		print("Not found")
		return
	}

	for left <= right {
		mid := (left + right) / 2

		if arr[mid] == searchValue {
			print("Found at position: ")
			println(mid)
			return
		} else if arr[mid] < searchValue {
			left = mid + 1
		} else {
			right = mid - 1
		}
	}

	println("Not found")
}
