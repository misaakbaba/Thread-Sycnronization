import sys



def write_file(len):
    with open('line.txt', 'w') as file:
        for i in range(len):
            file.write("This is Line ")
            file.write(str(i))
            file.write('\n')


if __name__ == '__main__':
    length = int(sys.argv[1])
    write_file(length)