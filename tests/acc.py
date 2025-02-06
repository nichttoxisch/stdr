with open("data/pride_and_prejudice.txt", 'r', encoding='utf-8') as f:
    content = f.read()
    words = content.split()

    dic = {}
    for word in words:
        word = word.lower()
        if word in dic:
            dic[word] += 1
        else:
            dic[word] = 1

    acc = list(dic.items());

    acc = sorted(acc, key=lambda x: x[1], reverse=True)

    print(acc[:10])