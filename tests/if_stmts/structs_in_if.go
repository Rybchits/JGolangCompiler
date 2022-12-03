package main

type Person struct {
	name        string
	age         int
	city, phone string
}

func (p Person) isMike() bool {
	return p.name == "Mike"
}

func isMike2(p Person) bool {
	return p.name == "Mike"
}

func main() {
	ivan := Person{name: "Ivan"}
	if isMike2(Person{name: "Mike"}) {
		print("точно майк")
	} else if ivan.isMike() {
		print("не точно майк")
	} else if Person{"Ivan"} {
		print("не точно майк")
	}
}
