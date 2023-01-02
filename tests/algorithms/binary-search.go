package main

func printArray(array []int) {
	for i := 0; i < lenArrayInt(array); i++ {
		printInt(array[i])
	}
}

func main() {
	searchValue := 0

	arr := []int{1, 5, 100, 0, -100, 15, 4, 102, 30, 1000}

	printArray(arr)

	// Sort the numbers
	var tmp = 0
	for i := 0; i < lenArrayInt(arr); i++ {
		for j := 0; j < lenArrayInt(arr)-1; j++ {
			if arr[j] > arr[j+1] {
				tmp = arr[j]
				arr[j] = arr[j+1]
				arr[j+1] = tmp
			}
		}
	}

	printArray(arr)

	left := 0
	right := lenArrayInt(arr) - 1

	if right < left {
		printString("Not found")
		return
	}

	for left <= right {
		mid := (left + right) / 2

		if arr[mid] == searchValue {
			printString("Found at position: ")
			printInt(mid)
			return
		} else if arr[mid] < searchValue {
			left = mid + 1
		} else {
			right = mid - 1
		}
	}

	printString("Not found")
}
