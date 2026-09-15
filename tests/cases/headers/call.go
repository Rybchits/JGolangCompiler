package main

type T struct{ X int }

func check(value T) bool { return true }
func main() {
	if check(T{X: 1}) {
		println(1)
	}
	for check(T{X: 1}) {
		break
	}
}
