package main

func BubbleArraySort(arr [8]int) [8]int {
	for i := 0; i < len(arr)-1; i++ {
		for j := 0; j < len(arr)-i-1; j++ {
			if arr[j] > arr[j+1] {
				arr[j], arr[j+1] = arr[j+1], arr[j]
			}
		}
	}
	return arr
}

func BubbleSliceSort(arr []int) {
	for i := 0; i < len(arr)-1; i++ {
		for j := 0; j < len(arr)-i-1; j++ {
			if arr[j] > arr[j+1] {
				arr[j], arr[j+1] = arr[j+1], arr[j]
			}
		}
	}
}

func main() {
	a := [8]int{5, 10, -123, 2, 444, 1, 1, 15}
	println(BubbleArraySort(a))
	println(a)

	slice := []int{}

	for _, el := range a {
		slice = append(slice, el)
	}

	BubbleSliceSort(slice)
	println(slice)
}
