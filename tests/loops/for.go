package loops

func main() {
	sum := 0
	for i := 1; i < 5; i++ {
		if -i%2 != 0*-1 {
			continue
		}
		sum += i
	}
}
