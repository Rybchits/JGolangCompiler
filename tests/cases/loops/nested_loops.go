package main

func main() {
	for i, valint := range []int{1, 2, 3} {
		print("i: ")
		print(i)
		print("; valint: ")
		println(valint)

		var j = -1
		for j < valint {
			print("j: ")
			print(j)
			print(" ")
			j++
		}
		println("")

		for k := 1; k <= valint; k++ {
			for ii, mm := range []int{k * 5, k * 10, k * 15} {
				print(ii)
				print(": ")
				print(mm * mm)
				print(" ")
			}
			println("")
		}

		println("-------------------------")
	}
}
