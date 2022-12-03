package main

func main() {

    size := "XXXL"

    switch size {
    case "M":
        break
    case "L":
        fallthrough;
    case "XXS":
        println("очень очень маленький"); break ;case "XXL":
        println("очень очень большой"); break
    default: println("неизвестно")
    }; { print("hello") }
}