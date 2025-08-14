import argparse
import csv

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--interval_high', type=float, required=True)
    parser.add_argument('--interval_low', type=float, required=True)
    parser.add_argument('--i_high', type=float, required=True)
    parser.add_argument('--i_low', type=float, required=True)
    parser.add_argument('--v_high', type=float, required=True)
    parser.add_argument('--v_low', type=float, required=True)
    args = parser.parse_args()

    counter = 0

    csvfile = open('script-square-wave.csv', 'w', newline='')
    writer = csv.writer(csvfile, delimiter=',',
                        quotechar='|', quoting=csv.QUOTE_MINIMAL)
    writer.writerow(['Step', 'Period', 'V(10/20V)', 'I(10/5A)'])

    for _ in range(10):
        writer.writerow([2*counter, '{}sec'.format(args.interval_high), args.v_high, args.i_high])
        writer.writerow([2*counter+1, '{}sec'.format(args.interval_low), args.v_low, args.i_low])
        counter += 1

if __name__ == '__main__':
    main()
