import http from 'k6/http';
import { check, sleep } from 'k6';

export const options = {
  vus: 10,
  duration: '45s',
  thresholds: {
    http_req_failed: ['rate<0.01'],
    http_req_duration: ['p(95)<400'],
  },
};

export default function () {
  const payload = JSON.stringify({
    title: `osipov-${__VU}-${__ITER}`,
    assignee: 'osipov',
    priority: 3,
    days_left: 5,
  });
  const params = { headers: { 'Content-Type': 'application/json' } };
  const created = http.post('http://localhost:8080/tasks', payload, params);
  check(created, { 'created 201': (r) => r.status === 201 });
  const listed = http.get('http://localhost:8080/tasks');
  check(listed, { 'list 200': (r) => r.status === 200 });
  sleep(1);
}
