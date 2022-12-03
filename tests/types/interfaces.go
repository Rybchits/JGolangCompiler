package main

type talkable interface{ talk() string; talkPhrase(string) string; }


type animal interface {
	breathe()
	walk()
}

type lion struct {
	age int
}

func (l lion) breathe() {
	println("Lion breathes")
}

func (l lion) walk() {
	println("Lion walk")
}

type dog struct {
	age int
}

func (l dog) breathe() {
	println("Dog breathes")
}

func (l dog) walk() {
	println("Dog walk")
}

func main() {
	/*var a animal

	a = lion{age: 10}
	a.breathe()
	a.walk()

	a = dog{age: 5}
	a.breathe()
	a.walk()*/
}
