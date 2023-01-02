package main

func BubbleSort(arr []int) {
	for i := 0; i < lenArrayInt(arr); i++ {
		for j := 0; j < lenArrayInt(arr)-1-i; j++ {
			if arr[j] > arr[j+1] {
				arr[j], arr[j+1] = arr[j+1], arr[j]
			}
		}
	}
}

func main() {
	a := []int{5, 3, 4, 7, 8, 9}
	BubbleSort(a)

	for el := 0; el < lenArrayInt(a); el++ {
	    printInt(el)
	}
}
