package main

/*
	type GlobalStruct struct {
		GlobalStruct
		name string
	}

func (g *GlobalStruct) main() string {

		type GlobalStruct struct {
			age int
		}
		var a = GlobalStruct{1}
		a.age = 2
		return g.name
	}

var temp = GlobalStruct{"hpoy"}
*/
func main() {
	type GlobalStruct struct {
		age int
	}

	var g = GlobalStruct{age: 1}
	print(g.age+5, g.name)
}
