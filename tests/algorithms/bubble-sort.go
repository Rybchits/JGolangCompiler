package main

func BubbleSort(arr []int) {
	for i := 0; i < len(arr)-1; i++ {
		for j := 0; j < len(arr)-i-1; j++ {
			if arr[j] > arr[j+1] {
				arr[j], arr[j+1] = arr[j+1], arr[j]
			}
		}
	}
}

func main() {
	a := []int{5, 10, -123, 2, 444, 1, 1, 15} //[-123 1 1 2 5 10 15 444]
	BubbleSort(a)                             // [-123 -123 -123 1 1 1 1 15]
	print(a)
}
