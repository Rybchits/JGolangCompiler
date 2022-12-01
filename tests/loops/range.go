package loops

func main() {
    strings := []string{"hello", "world"}
    for i, s := range strings {
	    println(i, s)
    }
}