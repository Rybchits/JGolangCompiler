package main

type GlobalStruct struct {
	name string
	age  int
}

func (g *GlobalStruct) main() string {

	type GlobalStruct struct {
		age int
	}
	var a = GlobalStruct{1}
	a.age = 2
	return g.name
}

func main() {

	var g = GlobalStruct{age: 1}
	a := true
	//if a {
	//	g.main()
	//}
}
