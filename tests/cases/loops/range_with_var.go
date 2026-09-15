package main

type Person struct {
	name  string
	array []int
}

func main() {
	var i, s int
	for i, s = range (Person{"name", []int{1, 2, 3}}).array {
		println(i, s)
	}
}
