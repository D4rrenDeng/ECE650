import sys
import re

# network maps the street to segment IDs
network = dict()
vertex_position_index = dict()
# segment maps the segment IDs to point coordinates
segment = dict()
vertex_count = 1
segment_count = 1

def check_pattern(points):
    pattern = re.compile("^([(][-]?[0-9]+[,][-]?[0-9]+[)])+$")
    if (pattern.match(points)):
        return True
    else:
        return False

def check_intersection(segment1, segment2):
    # the algorithm used for checking intersection is inspired by https://en.m.wikipedia.org/wiki/Line%E2%80%93line_intersection#:~:text=A%20necessary%20condition%20for%20two,sense%20of%20having%20zero%20volume
    # check if parallel or coincident
    x1 = segment[segment1][0]
    y1 = segment[segment1][1]
    x2 = segment[segment1][2]
    y2 = segment[segment1][3]
    x3 = segment[segment2][0]
    y3 = segment[segment2][1]
    x4 = segment[segment2][2]
    y4 = segment[segment2][3]
    if ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4)) == 0:
        return False
    t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4))
    u = ((x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2)) / ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4))
    if t >= 0 and t <= 1 and u >= 0 and u <= 1:
        return True
    else:
        return False

def add_vertices_tvalue(segment1, segment2):
    x1 = segment[segment1][0]
    y1 = segment[segment1][1]
    x2 = segment[segment1][2]
    y2 = segment[segment1][3]
    x3 = segment[segment2][0]
    y3 = segment[segment2][1]
    x4 = segment[segment2][2]
    y4 = segment[segment2][3]
    # only add the vertex's t and u values to the list for now instead of adding the vertex's coordinates, t is for segment1, u is for segment2
    t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4))
    u = ((x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2)) / ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4))
    if not segment1 in vertex_position_index:
        vertex_position_index[segment1] = [0, 1]
    vertex_position_index[segment1].append(t)
    vertex_position_index[segment1] = list(dict.fromkeys(vertex_position_index[segment1]))
    vertex_position_index[segment1].sort()
    if not segment2 in vertex_position_index:
        vertex_position_index[segment2] = [0, 1]
    vertex_position_index[segment2].append(u)   
    vertex_position_index[segment2] = list(dict.fromkeys(vertex_position_index[segment2]))
    vertex_position_index[segment2].sort()

def generate_graph():
    global vertex_count
    vertices = dict()
    edges = []
    for key, values in vertex_position_index.items():
        x1 = segment[key][0]
        y1 = segment[key][1]
        x2 = segment[key][2]
        y2 = segment[key][3]
        for i in range(len(values) - 1):
            t1 = values[i]
            t2 = values[i + 1]
            vx1 = round(x1 + t1 * (x2 - x1),3)
            vy1 = round(y1 + t1 * (y2 - y1),3)
            vx2 = round(x1 + t2 * (x2 - x1),3)
            vy2 = round(y1 + t2 * (y2 - y1),3)
            if not (vx1, vy1) in vertices:
                vertices[(vx1, vy1)] = "VV" + str(vertex_count)
                vertex_count += 1
            if not (vx2, vy2) in vertices:
                vertices[(vx2, vy2)] = "VV" + str(vertex_count)
                vertex_count += 1
            edges.append((vertices[(vx1, vy1)], vertices[(vx2, vy2)]))
    print("V = {")
    for key, value in vertices.items():
        print(' ' + value + ": " + "({0:.2f},{1:.2f})".format(key[0], key[1]))
    print("}")
    print("E = {")
    for i in range(len(edges)):
        if i < len(edges) - 1:
            print(" <" + edges[i][0] + "," + edges[i][1] + ">,")
        else:
            print(" <" + edges[i][0] + "," + edges[i][1] + ">")
    print("}")

def main():
    global segment_count
    while True:
        try:
            line = sys.stdin.readline()
            if line in (None, '', '\n'):
                raise EOFError
            line = line.replace('\n', '')
            line_list = line.lstrip().split('"')
            r = re.compile("[-]?[0-9]+")

            if len(line_list) == 3:
                command = line_list[0]
                # convert street names to lower cases since they are case insensitive
                street_name = line_list[1].replace('"', '').lower()
                points = line_list[2]

                # check street name
                if re.match("^[a-zA-Z ]+$", street_name):
                    pass
                else:
                    print("Error: illegal streetname!")
                    continue

                if(command[len(command) - 1] != ' '):
                    print("Error: missing space between command!")
                    continue
                command = command.replace(' ', '')
                # remove command
                if command == 'r':
                    points.replace(' ', '')
                    if points != '':
                        print("Error: unrecognized command!")
                        continue
                    if street_name in network:
                        segmentID = network[street_name]
                        for ID in segmentID:
                            if ID in segment:
                                segment.pop(ID)
                            if ID in vertex_position_index:
                                vertex_position_index.pop(ID)
                        del network[street_name]
                    else:
                        print("Error: street not found!")
                    continue

                if (points == ''):
                    print("Error: empty coordinates!")
                    continue
                # add or change
                if points[0] != ' ':
                    print("Error: missing space between command!")
                    continue
                points = points.replace(' ', '')
                if command == 'a':
                    if (not check_pattern(points)):
                        print("Error: coordinates were in the wrong format!")
                        continue
                    # add the street to the network
                    if street_name in network:
                        print("Error: street already exists!")
                        continue
                    network[street_name] = []
                    coordinates = r.findall(points)
                    for i in range(0, len(coordinates) - 3, 2):
                        x1 = int(coordinates[i])
                        y1 = int(coordinates[i + 1])
                        x2 = int(coordinates[i + 2])
                        y2 = int(coordinates[i + 3])
                        segment[segment_count] = (x1, y1, x2, y2)
                        network[street_name].append(segment_count)
                        segment_count += 1
                elif command == 'c':
                    if (not check_pattern(points)):
                        print("Error: coordinates were in the wrong format!")
                        continue
                    # change the street in the network
                    if not street_name in network:
                        print("Error: street not found!")
                        continue
                    segmentID = network[street_name]
                    for ID in segmentID:
                        if ID in segment:
                            segment.pop(ID)
                        if ID in vertex_position_index:
                            vertex_position_index.pop(ID)
                    network[street_name] = []
                    coordinates = r.findall(points)
                    for i in range(0, len(coordinates) - 3, 2):
                        x1 = int(coordinates[i])
                        y1 = int(coordinates[i + 1])
                        x2 = int(coordinates[i + 2])
                        y2 = int(coordinates[i + 3])
                        segment[segment_count] = (x1, y1, x2, y2)
                        network[street_name].append(segment_count)
                        segment_count += 1
                else:
                    print("Error: unrecognized command!")
                    continue
            elif len(line_list) == 1:
                command = line_list[0].replace(' ', '')
                if command == 'g':
                    vertex_position_index.clear()
                    for key1, street1 in network.items():
                        for key2, street2 in network.items():
                            if (not key1 == key2):
                                for m in range(len(street1)):
                                    for n in range(len(street2)):
                                        if check_intersection(network[key1][m], network[key2][n]):
                                            add_vertices_tvalue(network[key1][m], network[key2][n])
                    generate_graph()
                else:
                    print("Error: unrecognized command!")
                    continue
            else:
                print("Error: unrecognized command!")
                continue
        except KeyboardInterrupt:
            sys.exit(1)
        except EOFError:
            break
        except Exception as e:
            print("Error: " + str(e))
            continue

    sys.exit(0)

if __name__ == '__main__':
    main()
